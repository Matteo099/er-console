#include "InputSystem.h"

#include "log/Logger.h"

InputSystem::InputSystem()
{
    subsystems.reserve(16);
}

InputSystem::~InputSystem()
{
    reset();
}

bool InputSystem::begin(const InputConfig &config)
{
    reset();
    auto& i2c = I2CBusManager::bus();

    if (config.channels.enabled)
    {
        channelSystem = new ChannelSystem();
        nativeChannel = new NativeChannelProvider();
        channelSystem->registerProvider(nativeChannel);
        Log::debug(Log::INPUT_TAG, "NativeChannelProvider registered");

        if (config.channels.mcp1Enabled)
        {
            mcpDriver1 = new MCP23017Driver(i2c, config.channels.mcp1Base, config.channels.mcp1Address);
            mcp1 = new MCPChannelProvider(*mcpDriver1, config.channels.mcp1Base);
            channelSystem->registerProvider(mcp1);
            Log::debug(Log::INPUT_TAG, "MCP23017Driver 1 registered at base %d", config.channels.mcp1Base);
        }

        if (config.channels.mcp2Enabled)
        {
            mcpDriver2 = new MCP23017Driver(i2c, config.channels.mcp2Base, config.channels.mcp2Address);
            mcp2 = new MCPChannelProvider(*mcpDriver2, config.channels.mcp2Base);
            channelSystem->registerProvider(mcp2);
            Log::debug(Log::INPUT_TAG, "MCP23017Driver 2 registered at base %d", config.channels.mcp2Base);
        }

        if (config.channels.muxEnabled)
        {
            muxDriver = new CD74HC4067Driver(config.channels.muxCsPin, config.channels.muxS0Pin, config.channels.muxS1Pin, config.channels.muxS2Pin, config.channels.muxS3Pin);
            mux1 = new MuxChannelProvider(*muxDriver, config.channels.muxBase);
            channelSystem->registerProvider(mux1);
            Log::debug(Log::INPUT_TAG, "CD74HC4067Driver registered at base %d", config.channels.muxBase);
        }

        if (!channelSystem->init())
        {
            destroyChannels();

            Log::error(Log::INPUT_TAG, "Failed to initialize ChannelSystem");
        }
        else
        {
            channelInitialized = true;
            subsystems.push_back(channelSystem);
            Log::debug(Log::INPUT_TAG, "ChannelSystem initialized");
        }
    }
    if (config.power.enabled)
    {
        powerDriver = new INA219Driver(i2c, config.power.address);
        powerSystem = new PowerSystem(*powerDriver);
        if (!powerSystem->init())
        {
            destroyPower();

            Log::error(Log::INPUT_TAG, "Failed to initialize PowerSystem");
        }
        else
        {
            powerInitialized = true;
            Log::debug(Log::INPUT_TAG, "PowerSystem initialized");
            subsystems.push_back(powerSystem);
        }
    }
    if (config.rfid.enabled)
    {
        rfidDriver = new MFRC522Driver(config.rfid.ssPin, config.rfid.rstPin);
        rfidSystem = new RFIDSystem(*rfidDriver);
        if (!rfidSystem->init())
        {
            destroyRfid();

            Log::error(Log::INPUT_TAG, "Failed to initialize RFIDSystem");
        }
        else
        {
            rfidInitialized = true;
            Log::debug(Log::INPUT_TAG, "RFIDSystem initialized");
            subsystems.push_back(rfidSystem);
        }
    }
    if (config.oled.enabled)
    {
        oledDriver = new SSD1306Driver(config.oled.width, config.oled.height, i2c, config.oled.address);
        oledSystem = new OLEDSystem(*oledDriver);
        if (!oledSystem->init())
        {
            destroyOled();

            Log::error(Log::INPUT_TAG, "Failed to initialize OLEDSystem");
        }
        else
        {
            oledInitialized = true;
            Log::debug(Log::INPUT_TAG, "OLEDSystem initialized");
            subsystems.push_back(oledSystem);
        }
    }
    if (config.rgb.enabled)
    {
        rgbLedSystem = new RgbLedSystem();
        if (config.rgb.usePCA9685)
        {
            rgbExpander = new PCA9685Driver(i2c, config.rgb.address);
            rgbLedSystem->registerDriver(rgbExpander);
        }
        if (!rgbLedSystem->init())
        {
            destroyRgb();

            Log::error(Log::INPUT_TAG, "Failed to initialize RgbLedSystem");
        }
        else
        {
            rgbInitialized = true;
            subsystems.push_back(rgbLedSystem);
            Log::debug(Log::INPUT_TAG, "RgbLedSystem initialized");
        }
    }
    if (config.sevenSegment.enabled)
    {
        sevenSegDriver = new MAX7219Driver(config.sevenSegment.csPin, config.sevenSegment.digits);
        sevenSegmentSystem = new SevenSegmentSystem();
        sevenSegmentSystem->registerDriver(sevenSegDriver);
        if (!sevenSegmentSystem->init())
        {
            destroySevenSegment();

            Log::error(Log::INPUT_TAG, "Failed to initialize SevenSegmentSystem");
        }
        else
        {
            sevenSegmentInitialized = true;
            Log::debug(Log::INPUT_TAG, "SevenSegmentSystem initialized");
            subsystems.push_back(sevenSegmentSystem);
        }
    }
    if (config.relays.enabled)
    {
        if (!channelInitialized)
        {
            Log::error(
                Log::INPUT_TAG,
                "RelaySystem requires ChannelSystem to be enabled");

            return false;
        }
        relaySystem = new RelaySystem(channelSystem);
        if (!relaySystem->init())
        {
            destroyRelay();
            Log::error(Log::INPUT_TAG, "Failed to initialize RelaySystem");
        }
        else
        {
            relayInitialized = true;
            subsystems.push_back(relaySystem);
            Log::debug(Log::INPUT_TAG, "RelaySystem initialized");
        }
    }
    return true;
}

void InputSystem::shutdown()
{
    Log::info(Log::INPUT_TAG, "Shutting down InputSystem");
    reset();
}

void InputSystem::destroyChannels()
{
    if (channelSystem)
    {
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

void InputSystem::destroyPower()
{
    if (powerSystem)
    {
        powerSystem->shutdown();
    }

    delete powerSystem;
    powerSystem = nullptr;

    delete powerDriver;
    powerDriver = nullptr;

    powerInitialized = false;
}

void InputSystem::destroyRfid()
{
    if (rfidSystem)
    {
        rfidSystem->shutdown();
    }

    delete rfidSystem;
    rfidSystem = nullptr;

    delete rfidDriver;
    rfidDriver = nullptr;

    rfidInitialized = false;
}

void InputSystem::destroyOled()
{
    if (oledSystem)
    {
        oledSystem->shutdown();
    }

    delete oledSystem;
    oledSystem = nullptr;

    delete oledDriver;
    oledDriver = nullptr;

    oledInitialized = false;
}

void InputSystem::destroyRgb()
{
    if (rgbLedSystem)
    {
        rgbLedSystem->shutdown();
    }

    delete rgbLedSystem;
    rgbLedSystem = nullptr;

    delete rgbExpander;
    rgbExpander = nullptr;

    rgbInitialized = false;
}

void InputSystem::destroySevenSegment()
{
    if (sevenSegmentSystem)
    {
        sevenSegmentSystem->shutdown();
    }

    delete sevenSegmentSystem;
    sevenSegmentSystem = nullptr;

    delete sevenSegDriver;
    sevenSegDriver = nullptr;

    sevenSegmentInitialized = false;
}

void InputSystem::destroyRelay()
{
    if (relaySystem)
    {
        relaySystem->shutdown();
    }

    delete relaySystem;
    relaySystem = nullptr;

    relayInitialized = false;
}

void InputSystem::reset()
{
    subsystems.clear();

    destroyChannels();
    destroyPower();
    destroyRfid();
    destroyOled();
    destroyRgb();
    destroySevenSegment();
    destroyRelay();
}

void InputSystem::init(void *params)
{
    (void)params;

    if (!i2cManager.init(I2C_SDA_PIN,
                         I2C_SCL_PIN,
                         I2C_FREQUENCY))
    {
        Log::error(Log::INPUT_TAG, "Failed to initialize I2C bus");
        return;
    }

    Log::info(Log::INPUT_TAG, "InputSystem initialization completed (nothing to do)");
}

void InputSystem::update()
{
    for (auto *system : subsystems)
    {
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
PowerSystem *InputSystem::power()
{
    if (!powerInitialized)
    {
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
RFIDSystem *InputSystem::rfid()
{
    if (!rfidInitialized)
    {
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
OLEDSystem *InputSystem::oled()
{
    if (!oledInitialized)
    {
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
ChannelSystem *InputSystem::channels()
{
    if (!channelInitialized)
    {
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
RelaySystem *InputSystem::relays()
{
    if (!relayInitialized)
    {
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
RgbLedSystem *InputSystem::rgb()
{
    if (!rgbInitialized)
    {
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
SevenSegmentSystem *InputSystem::sevenSegment()
{
    if (!sevenSegmentInitialized)
    {
        Log::warn(Log::INPUT_TAG, "SevenSegmentSystem not initialized");

        return nullptr;
    }

    return sevenSegmentSystem;
}