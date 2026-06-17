#pragma once

#include <vector>

#include "core/SystemService.h"

#include "subsystems/ISubsystem.h"
#include "InputConfig.h"

#include "drivers/input/power/INA219Driver.h"
#include "drivers/input/rfid/MFRC522Driver.h"
#include "drivers/input/oled/SSD1306Driver.h"
#include "drivers/input/channel/NativeChannelProvider.h"
#include "drivers/input/channel/MCPChannelProvider.h"
#include "drivers/input/channel/MuxChannelProvider.h"
#include "drivers/input/rgb/ESP32PwmRgbDriver.h"
#include "drivers/input/rgb/PCA9685Driver.h"
#include "drivers/input/sevensegment/MAX7219Driver.h"

#include "subsystems/PowerSystem.h"
#include "subsystems/RFIDSystem.h"
#include "subsystems/OLEDSystem.h"
#include "subsystems/ChannelSystem.h"
#include "subsystems/RelaySystem.h"
#include "subsystems/RgbLedSystem.h"
#include "subsystems/SevenSegmentSystem.h"

class InputSystem : public SystemService
{
public:
    InputSystem();
    ~InputSystem() override = default;

    void init(void *params = nullptr) override;
    void update() override;

    PowerSystem *power();
    RFIDSystem *rfid();
    OLEDSystem *oled();
    ChannelSystem *channels();
    RelaySystem *relays();
    RgbLedSystem *rgb();
    SevenSegmentSystem *sevenSegment();

private:
    void registerSubsystems();
    bool _init(const InputConfig &config);

private:
    bool powerInitialized = false;
    bool rfidInitialized = false;
    bool oledInitialized = false;
    bool channelInitialized = false;
    bool rgbInitialized = false;
    bool sevenSegmentInitialized = false;
    bool relayInitialized = false;

    std::vector<ISubsystem *> subsystems;

    INA219Driver powerDriver;
    MFRC522Driver rfidDriver;
    SSD1306Driver oledDriver;
    NativeChannelProvider nativeChannel;
    MCPChannelProvider mcp1;
    MCPChannelProvider mcp2;
    MuxChannelProvider mux1;
    ESP32PwmRgbDriver statusLed;
    PCA9685Driver rgbExpander;
    MAX7219Driver sevenSegDriver;

    PowerSystem powerSystem;
    RFIDSystem rfidSystem;
    OLEDSystem oledSystem;
    ChannelSystem channelSystem;
    RelaySystem relaySystem;
    RgbLedSystem rgbLedSystem;
    SevenSegmentSystem sevenSegmentSystem;
};