#pragma once

#include <stdint.h>


struct PowerConfig
{
    bool enabled = false;

    uint8_t address = 0x40;
};


struct RFIDConfig
{
    bool enabled = false;

    uint8_t ssPin = 10;

    uint8_t rstPin = 9;
};


struct OLEDConfig
{
    bool enabled = false;

    uint8_t width = 128;

    uint8_t height = 64;

    uint8_t address = 0x3C;
};



struct ChannelConfig
{
    bool enabled = false;
};



struct RGBConfig
{
    bool enabled = false;

    bool usePCA9685 = false;

    uint8_t address = 0x40;
};



struct SevenSegmentConfig
{
    bool enabled = false;

    uint8_t csPin = 5;

    uint8_t digits = 8;
};



struct RelayConfig
{
    bool enabled = false;
};



struct InputConfig
{
    PowerConfig power;

    RFIDConfig rfid;

    OLEDConfig oled;

    ChannelConfig channels;

    RGBConfig rgb;

    SevenSegmentConfig sevenSegment;

    RelayConfig relays;
};