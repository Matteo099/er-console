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

bool HardwareCheck::identityPin(uint8_t pin) {
    // Only these ADC1 pins are free on the current ESP32-S3 N16R8 console.
    return (pin == 1 || pin == 2) && roomPin(pin);
}

unsigned HardwareCheck::identityVoltage(unsigned resistanceOhms) {
    return uint64_t(IdentitySupplyMillivolts) * resistanceOhms /
           (uint64_t(IdentityPullupOhms) + resistanceOhms);
}

std::vector<HardwareIssue> HardwareCheck::inspect(const InputConfig& config, HardwareProbe& probe,
                                                  bool prepare) {
    auto issues = config.validate();
    if (!issues.empty())
        return issues;  // Never touch pins from an invalid configuration.
    if (config.connection.enabled) {
        const auto found = probe.identity(config.connection.pin);
        const auto resistance = config.connection.resistanceOhms;
        const auto lower = identityVoltage(resistance * (100 - IdentityTolerancePercent) / 100) -
                           IdentityAdcMarginMillivolts;
        const auto upper = identityVoltage(resistance * (100 + IdentityTolerancePercent) / 100) +
                           IdentityAdcMarginMillivolts;
        HardwareIssueKind kind;
        bool failed = true;
        if (found.spreadMillivolts > IdentityMaxSpreadMillivolts)
            kind = HardwareIssueKind::IdentityUnstable;
        else if (found.millivolts >= IdentityOpenMillivolts)
            kind = HardwareIssueKind::MissingIdentity;
        else if (found.millivolts <= IdentityShortMillivolts)
            kind = HardwareIssueKind::IdentityShort;
        else if (found.millivolts < lower || found.millivolts > upper)
            kind = HardwareIssueKind::WrongRoom;
        else
            failed = false;
        if (failed) {
            char label[96];
            snprintf(label, sizeof(label), "GPIO %u, %u mV; %u ohm", config.connection.pin,
                     found.millivolts, resistance);
            issues.push_back({kind, label});
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
