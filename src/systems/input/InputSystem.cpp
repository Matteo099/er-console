#include "InputSystem.h"

#include "log/Logger.h"
#include "drivers/bus/SPIBusManager.h"
#include "drivers/input/RoomHardwareProbe.h"
#include <algorithm>

InputSystem::InputSystem() {
    subsystems.reserve(16);
}

InputSystem::~InputSystem() {
    reset();
}

void InputSystem::requestBegin(const InputConfig& config) {
    TaskMutex::Guard lock(hardwareMutex);
    ++generation;
    queuedConfig = config;
    beginPending = true;
    checkPending = false;
    hardware = HardwareStatus{};
    hardware.state = HardwareState::Checking;
}

void InputSystem::requestHardwareCheck() {
    TaskMutex::Guard lock(hardwareMutex);
    if (hardware.state != HardwareState::Ready)
        return;
    ++generation;
    checkPending = true;
    hardware.state = HardwareState::Checking;
    hardware.issues.clear();
}

HardwareStatus InputSystem::hardwareStatus() const {
    TaskMutex::Guard lock(hardwareMutex);
    return hardware;
}

bool InputSystem::powerSnapshot(PowerReading& reading) {
    TaskMutex::Guard lock(lifecycleMutex);
    if (!powerInitialized || !powerSystem || !powerSystem->isSupported()) {
        reading = PowerReading{};
        return false;
    }
    reading = powerSystem->getMetrics();
    return true;
}

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
void InputSystem::injectHardwareFaultForDiagnostics(HardwareIssueKind kind) {
    TaskMutex::Guard guard(lifecycleMutex);
    uint32_t token;
    {
        TaskMutex::Guard lock(hardwareMutex);
        token = generation;
    }
    monitoring = false;
    publish(token, HardwareState::Fault, {{kind, "MCP23017 #1 (I2C 0x27)"}});
}
#endif

void InputSystem::publish(uint32_t token, HardwareState state, std::vector<HardwareIssue> issues) {
    TaskMutex::Guard lock(hardwareMutex);
    if (token != generation)
        return;
    hardware.state = state;
    hardware.issues = std::move(issues);
    if (state == HardwareState::Fault)
        for (const auto& item : hardware.issues)
            Log::warn(Log::INPUT_TAG, "Hardware check failed: kind=%u component=%s",
                      unsigned(item.kind), item.component.c_str());
}

bool InputSystem::begin(const InputConfig& config, uint32_t token) {
    TaskMutex::Guard guard(lifecycleMutex);
    reset();
    auto issues = config.validate();
    if (!issues.empty()) {
        publish(token, HardwareState::Fault, std::move(issues));
        return false;
    }
    activeConfig = config;
    connectionConfigured = config.connection.enabled;
    RoomHardwareProbe probe;
    issues = HardwareCheck::inspect(config, probe, true);
    if (!issues.empty()) {
        reset();
        publish(token, HardwareState::Fault, std::move(issues));
        return false;
    }
    const auto failed = [&](const char* name) {
        issues.push_back({HardwareIssueKind::Initialization, name});
    };
    auto& i2c = I2CBusManager::bus();
    auto& spi = SPIBusManager::bus();

    if (config.channels.enabled) {
        channelSystem = new ChannelSystem();
        std::vector<uint8_t> reserved;
        if (config.connection.enabled)
            reserved.push_back(config.connection.pin);
        if (config.rfid.enabled) {
            reserved.push_back(config.rfid.ssPin);
            reserved.push_back(config.rfid.rstPin);
        }
        if (config.sevenSegment.enabled)
            reserved.push_back(config.sevenSegment.csPin);
        if (config.channels.muxEnabled) {
            reserved.insert(reserved.end(), {config.channels.muxCsPin, config.channels.muxS0Pin,
                                             config.channels.muxS1Pin, config.channels.muxS2Pin,
                                             config.channels.muxS3Pin});
        }
        nativeChannel = new NativeChannelProvider(std::move(reserved));
        channelSystem->registerProvider(nativeChannel);
        Log::debug(Log::INPUT_TAG, "NativeChannelProvider registered");

        if (config.channels.mcp1Enabled) {
            mcpDriver1 =
                new MCP23017Driver(i2c, config.channels.mcp1Base, config.channels.mcp1Address);
            mcp1 = new MCPChannelProvider(*mcpDriver1, config.channels.mcp1Base);
            channelSystem->registerProvider(mcp1);
            Log::debug(Log::INPUT_TAG, "MCP23017Driver 1 registered at base %d",
                       config.channels.mcp1Base);
        }

        if (config.channels.mcp2Enabled) {
            mcpDriver2 =
                new MCP23017Driver(i2c, config.channels.mcp2Base, config.channels.mcp2Address);
            mcp2 = new MCPChannelProvider(*mcpDriver2, config.channels.mcp2Base);
            channelSystem->registerProvider(mcp2);
            Log::debug(Log::INPUT_TAG, "MCP23017Driver 2 registered at base %d",
                       config.channels.mcp2Base);
        }

        if (config.channels.muxEnabled) {
            muxDriver = new CD74HC4067Driver(config.channels.muxCsPin, config.channels.muxS0Pin,
                                             config.channels.muxS1Pin, config.channels.muxS2Pin,
                                             config.channels.muxS3Pin);
            mux1 = new MuxChannelProvider(*muxDriver, config.channels.muxBase);
            channelSystem->registerProvider(mux1);
            Log::debug(Log::INPUT_TAG, "CD74HC4067Driver registered at base %d",
                       config.channels.muxBase);
        }

        if (!channelSystem->init()) {
            destroyChannels();

            Log::error(Log::INPUT_TAG, "Failed to initialize ChannelSystem");
            failed("MCP23017 / GPIO");
        } else {
            channelInitialized = true;
            subsystems.push_back(channelSystem);
            Log::debug(Log::INPUT_TAG, "ChannelSystem initialized");
        }
    }
    if (config.power.enabled) {
        powerDriver = new INA219Driver(i2c, config.power.address);
        powerSystem = new PowerSystem(*powerDriver);
        if (!powerSystem->init()) {
            destroyPower();

            Log::error(Log::INPUT_TAG, "Failed to initialize PowerSystem");
            failed("INA219");
        } else {
            powerInitialized = true;
            Log::debug(Log::INPUT_TAG, "PowerSystem initialized");
            subsystems.push_back(powerSystem);
        }
    }
    if (config.rfid.enabled) {
        rfidDriver = new MFRC522Reader(spi, config.rfid.ssPin, config.rfid.rstPin);
        rfidSystem = new RFIDSystem(*rfidDriver);
        if (!rfidSystem->init()) {
            destroyRfid();

            Log::error(Log::INPUT_TAG, "Failed to initialize RFIDSystem");
            failed("MFRC522");
        } else {
            rfidInitialized = true;
            Log::debug(Log::INPUT_TAG, "RFIDSystem initialized");
            subsystems.push_back(rfidSystem);
        }
    }
    if (config.oled.enabled) {
        oledDriver =
            new SSD1306Driver(config.oled.width, config.oled.height, i2c, config.oled.address);
        oledSystem = new OLEDSystem(*oledDriver);
        if (!oledSystem->init()) {
            destroyOled();

            Log::error(Log::INPUT_TAG, "Failed to initialize OLEDSystem");
            failed("SSD1306");
        } else {
            oledInitialized = true;
            Log::debug(Log::INPUT_TAG, "OLEDSystem initialized");
            subsystems.push_back(oledSystem);
        }
    }
    if (config.rgb.enabled) {
        rgbLedSystem = new RgbLedSystem();
        if (config.rgb.usePCA9685) {
            rgbExpander = new PCA9685Driver(i2c, config.rgb.address);
            rgbLedSystem->registerDriver(rgbExpander);
        }
        if (!rgbLedSystem->init()) {
            destroyRgb();

            Log::error(Log::INPUT_TAG, "Failed to initialize RgbLedSystem");
            failed("PCA9685");
        } else {
            rgbInitialized = true;
            subsystems.push_back(rgbLedSystem);
            Log::debug(Log::INPUT_TAG, "RgbLedSystem initialized");
        }
    }
    if (config.sevenSegment.enabled) {
        sevenSegDriver =
            new MAX7219Driver(spi, config.sevenSegment.csPin, config.sevenSegment.digits);
        sevenSegmentSystem = new SevenSegmentSystem();
        sevenSegmentSystem->registerDriver(sevenSegDriver);
        if (!sevenSegmentSystem->init()) {
            destroySevenSegment();

            Log::error(Log::INPUT_TAG, "Failed to initialize SevenSegmentSystem");
            failed("MAX7219");
        } else {
            sevenSegmentInitialized = true;
            Log::debug(Log::INPUT_TAG, "SevenSegmentSystem initialized");
            subsystems.push_back(sevenSegmentSystem);
        }
    }
    if (config.relays.enabled) {
        if (!channelInitialized) {
            Log::error(Log::INPUT_TAG, "RelaySystem requires ChannelSystem to be enabled");

            failed("Relay / GPIO");
            reset();
            publish(token, HardwareState::Fault, std::move(issues));
            return false;
        }
        relaySystem = new RelaySystem(channelSystem);
        if (!relaySystem->init()) {
            destroyRelay();
            Log::error(Log::INPUT_TAG, "Failed to initialize RelaySystem");
            failed("Relay / GPIO");
        } else {
            relayInitialized = true;
            subsystems.push_back(relaySystem);
            Log::debug(Log::INPUT_TAG, "RelaySystem initialized");
        }
    }
    if (!issues.empty()) {
        reset();
        publish(token, HardwareState::Fault, std::move(issues));
        return false;
    }
    monitor.reset();
    monitoring = true;
    checkedAt = millis();
    publish(token, HardwareState::Ready);
    return true;
}

void InputSystem::shutdown() {
    {
        TaskMutex::Guard lock(hardwareMutex);
        ++generation;
        beginPending = checkPending = false;
        hardware = HardwareStatus{};
    }
    TaskMutex::Guard guard(lifecycleMutex);
    Log::info(Log::INPUT_TAG, "Shutting down InputSystem");
    reset();
}

void InputSystem::destroyChannels() {
    if (channelSystem) {
        channelSystem->shutdown();
    }

    delete mcp1;
    mcp1 = nullptr;

    delete mcpDriver1;
    mcpDriver1 = nullptr;

    delete mcp2;
    mcp2 = nullptr;

    delete mcpDriver2;
    mcpDriver2 = nullptr;

    delete mux1;
    mux1 = nullptr;

    delete muxDriver;
    muxDriver = nullptr;

    delete nativeChannel;
    nativeChannel = nullptr;

    delete channelSystem;
    channelSystem = nullptr;

    channelInitialized = false;
}

void InputSystem::destroyPower() {
    if (powerSystem) {
        powerSystem->shutdown();
    }

    delete powerSystem;
    powerSystem = nullptr;

    delete powerDriver;
    powerDriver = nullptr;

    powerInitialized = false;
}

void InputSystem::destroyRfid() {
    if (rfidSystem) {
        rfidSystem->shutdown();
    }

    delete rfidSystem;
    rfidSystem = nullptr;

    delete rfidDriver;
    rfidDriver = nullptr;

    rfidInitialized = false;
}

void InputSystem::destroyOled() {
    if (oledSystem) {
        oledSystem->shutdown();
    }

    delete oledSystem;
    oledSystem = nullptr;

    delete oledDriver;
    oledDriver = nullptr;

    oledInitialized = false;
}

void InputSystem::destroyRgb() {
    if (rgbLedSystem) {
        rgbLedSystem->shutdown();
    }

    delete rgbLedSystem;
    rgbLedSystem = nullptr;

    delete rgbExpander;
    rgbExpander = nullptr;

    rgbInitialized = false;
}

void InputSystem::destroySevenSegment() {
    if (sevenSegmentSystem) {
        sevenSegmentSystem->shutdown();
    }

    delete sevenSegmentSystem;
    sevenSegmentSystem = nullptr;

    delete sevenSegDriver;
    sevenSegDriver = nullptr;

    sevenSegmentInitialized = false;
}

void InputSystem::destroyRelay() {
    if (relaySystem) {
        relaySystem->shutdown();
    }

    delete relaySystem;
    relaySystem = nullptr;

    relayInitialized = false;
}

void InputSystem::reset() {
    monitoring = false;
    monitor.reset();
    subsystems.clear();
    // Relay shutdown still needs the channel providers.
    destroyRelay();
    destroyPower();
    destroyRfid();
    destroyOled();
    destroyRgb();
    destroySevenSegment();
    destroyChannels();
    if (connectionConfigured)
        gpio_reset_pin(static_cast<gpio_num_t>(activeConfig.connection.pin));
    if (activeConfig.rfid.enabled) {
        pinMode(activeConfig.rfid.ssPin, INPUT_PULLUP);
        pinMode(activeConfig.rfid.rstPin, INPUT);
    }
    connectionConfigured = false;
    activeConfig = InputConfig{};
}

void InputSystem::init(void* params) {
    (void)params;

    if (!i2cManager.init(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQUENCY)) {
        Log::error(Log::INPUT_TAG, "Failed to initialize I2C bus");
        return;
    }

    Log::info(Log::INPUT_TAG, "InputSystem initialization completed (nothing to do)");
}

void InputSystem::update() {
    bool initialize, check;
    uint32_t token;
    InputConfig config;
    {
        TaskMutex::Guard lock(hardwareMutex);
        initialize = beginPending;
        check = checkPending;
        token = generation;
        if (initialize)
            config = queuedConfig;
        beginPending = checkPending = false;
    }
    TaskMutex::Guard guard(lifecycleMutex);
    {
        TaskMutex::Guard lock(hardwareMutex);
        if (token != generation)
            return;
    }
    if (initialize) {
        begin(config, token);
        return;
    }
    if (!monitoring)
        return;
    if (check || uint32_t(millis() - checkedAt) >= 500) {
        checkedAt = millis();
        RoomHardwareProbe probe;
        auto issues = HardwareCheck::inspect(activeConfig, probe, false);
        const bool wrongRoom =
            std::any_of(issues.begin(), issues.end(), [](const HardwareIssue& item) {
                return item.kind == HardwareIssueKind::WrongRoom;
            });
        const bool fault = monitor.sample(issues.empty());
        if (!issues.empty() && (check || wrongRoom || fault)) {
            monitoring = false;
            publish(token, HardwareState::Fault, std::move(issues));
            return;
        }
        if (check && issues.empty())
            publish(token, HardwareState::Ready);
        if (!issues.empty())
            return;
    }
    for (auto* system : subsystems) {
        if (system)
            system->update();
    }
}

/**
 * Initialize:
 *
 * InputConfig cfg;
 *
 * cfg.power.enabled=true;
 * cfg.power.address=0x40;
 *
 * input.init(cfg);
 *
 *
 * Usage:
 *
 * auto p=input.power();
 *
 * if(p)
 * {
 *    auto m=p->metrics();
 * }
 */
PowerSystem* InputSystem::power() {
    if (!powerInitialized) {
        Log::warn(Log::INPUT_TAG, "PowerSystem not initialized");
        return nullptr;
    }

    return powerSystem;
}

/**
 * Initialize:
 *
 * cfg.rfid.enabled=true;
 * cfg.rfid.ssPin=5;
 * cfg.rfid.rstPin=27;
 *
 *
 * Usage:
 *
 * if(auto r=input.rfid())
 * {
 *     RFIDTag tag;
 *
 *     if(r->consumeTag(tag))
 *     {
 *        Serial.println(tag.uid);
 *     }
 * }
 */
RFIDSystem* InputSystem::rfid() {
    if (!rfidInitialized) {
        Log::warn(Log::INPUT_TAG, "RFIDSystem not initialized");

        return nullptr;
    }

    return rfidSystem;
}

/**
 * Initialize:
 *
 * cfg.oled.enabled=true;
 * cfg.oled.address=0x3C;
 *
 *
 * Usage:
 *
 * input.oled()
 *      ->showMessage(
 *          "READY");
 */
OLEDSystem* InputSystem::oled() {
    if (!oledInitialized) {
        Log::warn(Log::INPUT_TAG, "OLEDSystem not initialized");

        return nullptr;
    }

    return oledSystem;
}

/**
 * Initialize:
 *
 * cfg.channels.enabled=true;
 *
 *
 * Usage:
 * // Set pin 5 as OUTPUT (ESP32 GPIO)
 * input.channels()
 *     ->pinMode(
 *         5,
 *        OUTPUT);
 *
 * // Read pin 5 state (ESP32 GPIO)
 * bool state =
 *    input.channels()
 *       ->digitalRead(5);
 *
 * // Set pin 5 HIGH (MCP23017)
 * input.channels()
 *      ->digitalWrite(
 *          100,
 *          HIGH);
 *
 * // Read analog value from channel 203 (CD74HC4067)
 * int value =
 *   input.channels()
 *     ->analogRead(203);
 */
ChannelSystem* InputSystem::channels() {
    if (!channelInitialized) {
        Log::warn(Log::INPUT_TAG, "ChannelSystem not initialized");

        return nullptr;
    }

    return channelSystem;
}

/**
 * Initialize:
 *
 * cfg.relays.enabled=true;
 *
 *
 * Usage:
 * // Turn ON
 * input.relays()
 *      ->set(
 *          0,
 *          true);
 *
 * // Turn OFF
 * input.relays()
 *     ->set(
 *         0,
 *        false);
 *
 * // Toggle
 * input.relays()
 *    ->toggle(0);
 *
 * // Pulse (500ms pulse)
 * input.relays()
 *   ->pulse(
 *       0,
 *      500);
 */
RelaySystem* InputSystem::relays() {
    if (!relayInitialized) {
        Log::warn(Log::INPUT_TAG, "RelaySystem not initialized");

        return nullptr;
    }

    return relaySystem;
}

/**
 * Initialize:
 *
 * cfg.rgb.enabled=true;
 *
 *
 * Usage:
 * // set red color
 * input.rgb()
 *      ->setColor(
 *          0,
 *          {255,0,0});
 *
 * // set brightness to 50%
 * input.rgb()
 *      ->setBrightness(
 *         0,
 *        50);
 */
RgbLedSystem* InputSystem::rgb() {
    if (!rgbInitialized) {
        Log::warn(Log::INPUT_TAG, "RgbLedSystem not initialized");

        return nullptr;
    }

    return rgbLedSystem;
}

/**
 * Initialize:
 *
 * cfg.sevenSegment.enabled=true;
 * cfg.sevenSegment.csPin=5;
 *
 *
 * Usage:
 * // Show number
 * input.sevenSegment()
 *      ->display(
 *          0,
 *          "12.5");
 *
 * // Set brightness to 50%
 * input.sevenSegment()
 *     ->brightness(
 *          0,
 *          5);
 */
SevenSegmentSystem* InputSystem::sevenSegment() {
    if (!sevenSegmentInitialized) {
        Log::warn(Log::INPUT_TAG, "SevenSegmentSystem not initialized");

        return nullptr;
    }

    return sevenSegmentSystem;
}
