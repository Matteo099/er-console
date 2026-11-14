#include "HardwareCheck.h"
#include "config/BoardConfig.h"
#include <cstdio>

bool HardwareCheck::roomPin(uint8_t pin) {
    // ESP32-S3 N16R8: console buses, UART, flash, OPI PSRAM and boot pins are reserved.
    if (!(pin == 1 || pin == 2 || pin == 19 || (pin >= 38 && pin <= 42)))
        return false;
    for (int reserved :
         {SPI1_MOSI_PIN, SPI1_MISO_PIN,    SPI1_SCLK_PIN,      TFT_CS_PIN,    TFT_DC_PIN,
          TFT_RST_PIN,   TOUCH_CS_PIN,     TOUCH_IRQ_PIN,      SPI3_MISO_PIN, SPI3_MOSI_PIN,
          SPI3_SCLK_PIN, SD_CS_PIN,        SD_POWER_RELAY_PIN, I2S_DOUT_PIN,  I2S_BCLK_PIN,
          I2S_LRCLK_PIN, I2S_SHUTDOWN_PIN, I2C_SDA_PIN,        I2C_SCL_PIN,   NEOPIXEL_PIN})
        if (pin == reserved)
            return false;
    return true;
}

bool HardwareCheck::validRoomId(const std::string& rom) {
    if (rom.size() != 16 || rom.substr(0, 2) != "01")
        return false;
    uint8_t bytes[8]{};
    for (unsigned i = 0; i < 16; ++i) {
        const char c = rom[i];
        if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')))
            return false;
        bytes[i / 2] = bytes[i / 2] * 16 + (c <= '9' ? c - '0' : c - 'A' + 10);
    }
    uint8_t crc = 0;
    for (unsigned i = 0; i < 8; ++i) {
        uint8_t value = bytes[i];
        for (unsigned bit = 0; bit < 8; ++bit) {
            const bool mix = (crc ^ value) & 1;
            crc >>= 1;
            if (mix)
                crc ^= 0x8C;
            value >>= 1;
        }
    }
    return crc == 0;
}

std::vector<HardwareIssue> HardwareCheck::inspect(const InputConfig& config, HardwareProbe& probe,
                                                  bool prepare) {
    auto issues = config.validate();
    if (!issues.empty())
        return issues;  // Never touch pins from an invalid configuration.
    if (config.connection.enabled) {
        const auto found = probe.identity(config.connection.pin);
        if (!validRoomId(found) || found != config.connection.rom) {
            issues.push_back({validRoomId(found) ? HardwareIssueKind::WrongRoom
                                                 : HardwareIssueKind::MissingIdentity,
                              "DS2401"});
            return issues;  // Do not initialize outputs on the wrong room.
        }
    }
    const auto check = [&](bool enabled, uint8_t address, const char* name) {
        if (!enabled || probe.i2c(address))
            return;
        char label[64];
        snprintf(label, sizeof(label), "%s (I2C 0x%02X)", name, address);
        issues.push_back({HardwareIssueKind::MissingI2C, label});
    };
    check(config.power.enabled, config.power.address, "INA219");
    check(config.oled.enabled, config.oled.address, "OLED SSD1306");
    check(config.channels.enabled && config.channels.mcp1Enabled, config.channels.mcp1Address,
          "MCP23017 #1");
    check(config.channels.enabled && config.channels.mcp2Enabled, config.channels.mcp2Address,
          "MCP23017 #2");
    check(config.rgb.enabled && config.rgb.usePCA9685, config.rgb.address, "PCA9685");
    // A mismatched I2C profile must not drive CS/RST pins on another room.
    if (!issues.empty())
        return issues;
    if (config.rfid.enabled && !probe.rfid(config.rfid.ssPin, config.rfid.rstPin, prepare)) {
        char label[64];
        snprintf(label, sizeof(label), "RFID MFRC522 (CS %u, RST %u)", config.rfid.ssPin,
                 config.rfid.rstPin);
        issues.push_back({HardwareIssueKind::MissingRFID, label});
    }
    return issues;
}
