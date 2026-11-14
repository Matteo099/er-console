#pragma once

#include <stdint.h>
#include "HardwareStatus.h"
#include <ArduinoJson.h>

struct PowerConfig {
    bool enabled = false;
    uint8_t address = 0x40;
};

struct RFIDConfig {
    bool enabled = false;
    uint8_t ssPin = 41;
    uint8_t rstPin = 40;
    std::string spiBus = "sd";
};

struct OLEDConfig {
    bool enabled = false;
    uint8_t width = 128;
    uint8_t height = 64;
    uint8_t address = 0x3C;
};

struct ChannelConfig {
    bool enabled = false;
    bool mcp1Enabled = false;
    bool mcp2Enabled = false;
    bool muxEnabled = false;
    uint8_t mcp1Base = 100;
    uint8_t mcp1Address = 0x20;
    uint8_t mcp2Base = 120;
    uint8_t mcp2Address = 0x21;
    uint8_t muxBase = 200;
    uint8_t muxCsPin = 5;
    uint8_t muxS0Pin = 12;
    uint8_t muxS1Pin = 13;
    uint8_t muxS2Pin = 14;
    uint8_t muxS3Pin = 15;
};

struct RGBConfig {
    bool enabled = false;
    bool usePCA9685 = false;
    uint8_t address = 0x40;
};

struct SevenSegmentConfig {
    bool enabled = false;
    uint8_t csPin = 5;
    uint8_t digits = 8;
};

struct RelayConfig {
    bool enabled = false;
};

struct ConnectionConfig {
    bool enabled = false;
    uint8_t pin = 38;
    std::string rom;
};

struct InputConfig {
    PowerConfig power;
    RFIDConfig rfid;
    OLEDConfig oled;
    ChannelConfig channels;
    RGBConfig rgb;
    SevenSegmentConfig sevenSegment;
    RelayConfig relays;
    ConnectionConfig connection;
    std::vector<HardwareIssue> configurationIssues;
    static InputConfig parse(JsonVariantConst inputs);
    std::vector<HardwareIssue> validate() const;
};
