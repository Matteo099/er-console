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

struct MCPConfig {
    bool enabled = true;
    uint8_t base = 0;
    uint8_t address = 0;
};

struct ChannelConfig {
    bool enabled = false;
    std::vector<MCPConfig> mcp;
    bool muxEnabled = false;
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
    uint8_t pin = 1;
    unsigned resistanceOhms = 0;
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
