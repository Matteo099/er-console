#include "InputConfig.h"
#include "HardwareCheck.h"
#include "config/BoardConfig.h"
#include <algorithm>
#include <cstdio>

InputConfig InputConfig::parse(JsonVariantConst inputs) {
    InputConfig result;
    const auto issue = [&](const std::string& field,
                           HardwareIssueKind kind = HardwareIssueKind::Configuration) {
        if (result.configurationIssues.size() < 16)
            result.configurationIssues.push_back({kind, field});
    };
    if (inputs.isNull())
        return result;
    if (!inputs.is<JsonObjectConst>()) {
        issue("inputs");
        return result;
    }
    const auto object = [&](const char* name) -> JsonObjectConst {
        auto value = inputs[name];
        if (!value.isNull() && !value.is<JsonObjectConst>())
            issue(std::string("inputs.") + name);
        return value.as<JsonObjectConst>();
    };
    const auto flag = [&](JsonObjectConst object, const char* key, bool& target,
                          const std::string& name) {
        if (object[key].isNull())
            return;
        if (!object[key].is<bool>())
            issue(name + "." + key);
        else
            target = object[key].as<bool>();
    };
    const auto integer = [&](JsonObjectConst object, const char* key, uint8_t& target,
                             const std::string& name) {
        if (object[key].isNull())
            return;
        if (!object[key].is<unsigned>() || object[key].as<unsigned>() > 255)
            issue(name + "." + key);
        else
            target = object[key].as<unsigned>();
    };
    for (auto pair : inputs.as<JsonObjectConst>()) {
        const std::string name = pair.key().c_str();
        if (name == "power" || name == "rfid" || name == "oled" || name == "channels" ||
            name == "rgb" || name == "sevenSegment" || name == "relays" || name == "connection" ||
            name == "i2c")
            continue;
        if (!pair.value().is<JsonObjectConst>() || !pair.value()["enabled"].is<bool>())
            issue("inputs." + name);
        else if (pair.value()["enabled"].as<bool>())
            issue("inputs." + name, HardwareIssueKind::Unsupported);
    }
    auto power = object("power");
    flag(power, "enabled", result.power.enabled, "power");
    integer(power, "address", result.power.address, "power");
    auto rfid = object("rfid");
    flag(rfid, "enabled", result.rfid.enabled, "rfid");
    integer(rfid, "ssPin", result.rfid.ssPin, "rfid");
    integer(rfid, "rstPin", result.rfid.rstPin, "rfid");
    if (!rfid["spiBus"].isNull()) {
        if (!rfid["spiBus"].is<const char*>())
            issue("rfid.spiBus");
        else
            result.rfid.spiBus = rfid["spiBus"].as<const char*>();
    }
    auto oled = object("oled");
    flag(oled, "enabled", result.oled.enabled, "oled");
    integer(oled, "width", result.oled.width, "oled");
    integer(oled, "height", result.oled.height, "oled");
    integer(oled, "address", result.oled.address, "oled");
    auto channels = object("channels");
    flag(channels, "enabled", result.channels.enabled, "channels");
    flag(channels, "mcp1Enabled", result.channels.mcp1Enabled, "channels");
    flag(channels, "mcp2Enabled", result.channels.mcp2Enabled, "channels");
    flag(channels, "muxEnabled", result.channels.muxEnabled, "channels");
    integer(channels, "mcp1Base", result.channels.mcp1Base, "channels");
    integer(channels, "mcp2Base", result.channels.mcp2Base, "channels");
    integer(channels, "mcp1Address", result.channels.mcp1Address, "channels");
    integer(channels, "mcp2Address", result.channels.mcp2Address, "channels");
    integer(channels, "muxBase", result.channels.muxBase, "channels");
    integer(channels, "muxCsPin", result.channels.muxCsPin, "channels");
    integer(channels, "muxS0Pin", result.channels.muxS0Pin, "channels");
    integer(channels, "muxS1Pin", result.channels.muxS1Pin, "channels");
    integer(channels, "muxS2Pin", result.channels.muxS2Pin, "channels");
    integer(channels, "muxS3Pin", result.channels.muxS3Pin, "channels");
    for (auto pair : channels) {
        const std::string name = pair.key().c_str();
        if (name.compare(0, 3, "mcp") == 0 && name != "mcp1Enabled" && name != "mcp2Enabled" &&
            name.size() >= 7 && name.substr(name.size() - 7) == "Enabled") {
            if (!pair.value().is<bool>())
                issue("channels." + name);
            else if (pair.value().as<bool>())
                issue("channels." + name, HardwareIssueKind::Unsupported);
        }
    }
    auto rgb = object("rgb");
    flag(rgb, "enabled", result.rgb.enabled, "rgb");
    flag(rgb, "usePCA9685", result.rgb.usePCA9685, "rgb");
    integer(rgb, "address", result.rgb.address, "rgb");
    auto segment = object("sevenSegment");
    flag(segment, "enabled", result.sevenSegment.enabled, "sevenSegment");
    integer(segment, "csPin", result.sevenSegment.csPin, "sevenSegment");
    integer(segment, "digits", result.sevenSegment.digits, "sevenSegment");
    flag(object("relays"), "enabled", result.relays.enabled, "relays");
    auto connection = object("connection");
    flag(connection, "enabled", result.connection.enabled, "connection");
    integer(connection, "pin", result.connection.pin, "connection");
    if (!connection["resistanceOhms"].isNull()) {
        if (!connection["resistanceOhms"].is<unsigned>())
            issue("connection.resistanceOhms");
        else
            result.connection.resistanceOhms = connection["resistanceOhms"].as<unsigned>();
    }
    auto i2c = object("i2c");
    for (const auto& field : {std::pair<const char*, unsigned>{"sdaPin", I2C_SDA_PIN},
                              {"sclPin", I2C_SCL_PIN},
                              {"frequency", I2C_FREQUENCY}}) {
        auto value = i2c[field.first];
        if (!value.isNull() && (!value.is<unsigned>() || value.as<unsigned>() != field.second))
            issue(std::string("i2c.") + field.first, HardwareIssueKind::Unsupported);
    }
    return result;
}

std::vector<HardwareIssue> InputConfig::validate() const {
    auto issues = configurationIssues;
    const auto add = [&](const std::string& name,
                         HardwareIssueKind kind = HardwareIssueKind::Configuration) {
        if (issues.size() < 16)
            issues.push_back({kind, name});
    };
    std::vector<uint8_t> addresses, pins, bases;
    const auto address = [&](uint8_t value, const char* name, unsigned low, unsigned high) {
        if (value < low || value > high ||
            std::find(addresses.begin(), addresses.end(), value) != addresses.end()) {
            char field[80];
            snprintf(field, sizeof(field), "%s (I2C 0x%02X)", name, value);
            add(field);
        }
        addresses.push_back(value);
    };
    const auto pin = [&](uint8_t value, const char* name) {
        if (!HardwareCheck::roomPin(value) ||
            std::find(pins.begin(), pins.end(), value) != pins.end()) {
            char field[80];
            snprintf(field, sizeof(field), "%s (GPIO %u)", name, value);
            add(field);
        }
        pins.push_back(value);
    };
    const auto base = [&](uint8_t value, const char* name) {
        bool valid = value > GPIO_MAX_PIN && value <= 240;
        for (uint8_t prior : bases)
            if (value < unsigned(prior) + 16 && prior < unsigned(value) + 16)
                valid = false;
        if (!valid)
            add(name);
        bases.push_back(value);
    };
    if (power.enabled)
        address(power.address, "INA219", 0x40, 0x4F);
    if (oled.enabled) {
        address(oled.address, "SSD1306", 0x3C, 0x3D);
        if (oled.width != 128 || (oled.height != 32 && oled.height != 64))
            add("oled.width/height");
    }
    if (channels.enabled) {
        if (channels.mcp1Enabled) {
            address(channels.mcp1Address, "MCP23017 #1", 0x20, 0x27);
            base(channels.mcp1Base, "channels.mcp1Base");
        }
        if (channels.mcp2Enabled) {
            address(channels.mcp2Address, "MCP23017 #2", 0x20, 0x27);
            base(channels.mcp2Base, "channels.mcp2Base");
        }
        if (channels.muxEnabled) {
            base(channels.muxBase, "channels.muxBase");
            pin(channels.muxCsPin, "MUX CS");
            pin(channels.muxS0Pin, "MUX S0");
            pin(channels.muxS1Pin, "MUX S1");
            pin(channels.muxS2Pin, "MUX S2");
            pin(channels.muxS3Pin, "MUX S3");
        }
    } else if (channels.mcp1Enabled || channels.mcp2Enabled || channels.muxEnabled)
        add("channels.enabled");
    if (rfid.enabled) {
        pin(rfid.ssPin, "RFID CS");
        pin(rfid.rstPin, "RFID RST");
        if (rfid.spiBus != "sd")
            add("rfid.spiBus", HardwareIssueKind::Unsupported);
    }
    if (rgb.enabled) {
        if (rgb.usePCA9685)
            address(rgb.address, "PCA9685", 0x40, 0x77);
        else
            add("rgb.usePCA9685", HardwareIssueKind::Unsupported);
    }
    if (sevenSegment.enabled) {
        pin(sevenSegment.csPin, "MAX7219 CS");
        if (sevenSegment.digits < 1 || sevenSegment.digits > 8)
            add("sevenSegment.digits");
    }
    if (relays.enabled && !channels.enabled)
        add("relays/channels.enabled");
    if (connection.enabled) {
        pin(connection.pin, "Room ADC");
        if (!HardwareCheck::identityPin(connection.pin))
            add("connection.pin (ADC1 GPIO 1/2)");
        if (connection.resistanceOhms < 150 || connection.resistanceOhms > 4700)
            add("connection.resistanceOhms (150-4700)");
    }
    return issues;
}
