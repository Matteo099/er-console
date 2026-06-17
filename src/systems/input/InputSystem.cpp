#include "InputSystem.h"

#include "log/Logger.h"

InputSystem::InputSystem() : powerDriver(),
                             rfidDriver(RFID_SS_PIN, RFID_RST_PIN),
                             oledDriver(128, 64, Wire, 0x3C),
                             nativeChannel(),
                             mcp1(100, 0x20),
                             mcp2(116, 0x21),
                             mux1(200, 34, 25, 26, 27, 14),
                             statusLed(21, 22, 23),
                             rgbExpander(0x40),
                             sevenSegDriver(5, 8),

                             powerSystem(powerDriver),
                             rfidSystem(rfidDriver),
                             oledSystem(oledDriver),
                             relaySystem(channelSystem)
{
    channelSystem.registerProvider(&nativeChannel);
    channelSystem.registerProvider(&mcp1);
    channelSystem.registerProvider(&mcp2);
    channelSystem.registerProvider(&mux1);
    rgbLedSystem.registerDriver(&statusLed);
    rgbLedSystem.registerDriver(&rgbExpander);
    sevenSegmentSystem.registerDriver(&sevenSegDriver);
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
    for (ISubsystem *subsystem : subsystems)
    {
        if (!subsystem)
            continue;

        subsystem->update();
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

PowerSystem &InputSystem::power()
{
    return powerSystem;
}

/*
Usage:

RFIDTag tag;
if(input.rfid().consumeTag(tag))
{
    Log::info(
        Log::APP_TAG,
        "Card scanned: %s",
        tag.uid.c_str());
}
 */
RFIDSystem &InputSystem::rfid()
{
    return rfidSystem;
}


/*
Usage:

input.oled().showMessage(
    "System Ready");
*/
OLEDSystem &InputSystem::oled()
{
    return oledSystem;
}

/*
Example usage

ESP32 GPIO:
input.channels()
     .pinMode(
          5,
          OUTPUT);
input.channels()
     .digitalWrite(
          5,
          HIGH);

MCP23017:
input.channels()
     .digitalWrite(
          100,
          HIGH);

CD74HC4067:
int sensor =
    input.channels()
         .analogRead(203);
*/
ChannelSystem &InputSystem::channels()
{
    return channelSystem;
}


/*
Usage

Register relays (during setup):
input.relays()
     .registerRelay(
        0,
        100,
        true);

Turn ON:
input.relays()
     .set(
        0,
        true);

OFF:
input.relays()
     .set(
        0,
        false);

Toggle:
input.relays()
     .toggle(0);

Pulse (500ms pulse):
input.relays()
     .pulse(
        0,
        500);
*/
RelaySystem &InputSystem::relays()
{
    return relaySystem;
}

/*
Usage:

// set red color
input.rgb()
     .setColor(
          0,
          {255,0,0});


// set brightness to 50%
input.rgb()
     .setBrightness(
          0,
          50);
*/
RgbLedSystem &InputSystem::rgb()
{
    return rgbLedSystem;
}

/*
Usage

Show number:
input.sevenSegment()
     .display(
        0,
        "1234");

Temperature:
input.sevenSegment()
     .display(
        0,
        "25.5");

Error:
input.sevenSegment()
     .display(
        0,
        "E-01");

Brightness:
input.sevenSegment()
     .brightness(
        0,
        5);
*/
SevenSegmentSystem &InputSystem::sevenSegment()
{
    return sevenSegmentSystem;
}