#include "InputSystem.h"

#include "log/Logger.h"

InputSystem::InputSystem()
{
}

bool InputSystem::_init(const InputConfig &config)
{
    subsystems.clear();
    if (config.channels.enabled)
    {
        channelSystem.init();
        channelInitialized = true;
        subsystems.push_back(&channelSystem);
    }
    if (config.power.enabled)
    {
        powerDriver = INA219Driver(Wire, config.power.address);
        powerSystem = PowerSystem(powerDriver);
        if (powerSystem.init())
        {
            powerInitialized = true;
            subsystems.push_back(&powerSystem);
        }
    }
    if (config.rfid.enabled)
    {
        rfidDriver = MFRC522Driver(config.rfid.ssPin, config.rfid.rstPin);
        rfidSystem = RFIDSystem(rfidDriver);
        if (rfidSystem.init())
        {
            rfidInitialized = true;
            subsystems.push_back(&rfidSystem);
        }
    }
    if (config.oled.enabled)
    {
        oledDriver = SSD1306Driver(config.oled.width, config.oled.height, Wire, config.oled.address);
        oledSystem = OLEDSystem(oledDriver);
        if (oledSystem.init())
        {
            oledInitialized = true;
            subsystems.push_back(&oledSystem);
        }
    }
    if (config.rgb.enabled)
    {
        rgbLedSystem.init();
        rgbInitialized = true;
        subsystems.push_back(&rgbLedSystem);
    }
    if (config.sevenSegment.enabled)
    {
        sevenSegmentSystem.init();
        sevenSegmentInitialized = true;
        subsystems.push_back(&sevenSegmentSystem);
    }
    if (config.relays.enabled)
    {
        relaySystem = RelaySystem(channelSystem);
        relaySystem.init();
        relayInitialized = true;
        subsystems.push_back(&relaySystem);
    }
    return true;
}

void InputSystem::init(void *params)
{
    (void)params;

    registerSubsystems();

    Log::info(Log::INPUT_TAG,
              "Initializing %u input subsystems",
              static_cast<unsigned>(subsystems.size()));

    for (ISubsystem *subsystem : subsystems)
    {
        if (!subsystem)
            continue;

        subsystem->init();
    }

    Log::info(Log::INPUT_TAG,
              "Input subsystem initialization completed");
}

void InputSystem::update()
{
    for (auto *system : subsystems)
    {
        if (system)
            system->update();
    }
}

void InputSystem::registerSubsystems()
{
    subsystems.clear();

    subsystems.reserve(7);

    subsystems.push_back(&powerSystem);
    subsystems.push_back(&rfidSystem);
    subsystems.push_back(&oledSystem);
    subsystems.push_back(&channelSystem);
    subsystems.push_back(&relaySystem);
    subsystems.push_back(&rgbLedSystem);
    subsystems.push_back(&sevenSegmentSystem);
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
        Log::warn(
            Log::INPUT_TAG,
            "PowerSystem not initialized");

        return nullptr;
    }

    return &powerSystem;
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
        Log::warn(
            Log::INPUT_TAG,
            "RFIDSystem not initialized");

        return nullptr;
    }

    return &rfidSystem;
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
        Log::warn(
            Log::INPUT_TAG,
            "OLEDSystem not initialized");

        return nullptr;
    }

    return &oledSystem;
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
        Log::warn(
            Log::INPUT_TAG,
            "ChannelSystem not initialized");

        return nullptr;
    }

    return &channelSystem;
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
        Log::warn(
            Log::INPUT_TAG,
            "RelaySystem not initialized");

        return nullptr;
    }

    return &relaySystem;
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
        Log::warn(
            Log::INPUT_TAG,
            "RgbLedSystem not initialized");

        return nullptr;
    }

    return &rgbLedSystem;
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
        Log::warn(
            Log::INPUT_TAG,
            "SevenSegmentSystem not initialized");

        return nullptr;
    }

    return &sevenSegmentSystem;
}