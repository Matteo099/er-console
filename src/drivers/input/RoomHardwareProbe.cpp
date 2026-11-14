#include "RoomHardwareProbe.h"
#include "drivers/bus/I2CBusManager.h"
#include "drivers/bus/I2CBusLock.h"
#include "drivers/bus/SPIBusManager.h"
#include "drivers/bus/SPIBusLock.h"
#include <Arduino.h>
#include <algorithm>

bool RoomHardwareProbe::i2c(uint8_t address) {
    I2CBusLock::Guard lock;
    if (!I2CBusManager::mutex())
        return false;
    auto& wire = I2CBusManager::bus();
    wire.beginTransmission(address);
    return wire.endTransmission() == 0;
}

bool RoomHardwareProbe::rfid(uint8_t chipSelect, uint8_t reset, bool prepare) {
    SPIBusLock::Guard lock;
    auto& spi = SPIBusManager::bus();
    if (!spi.bus())
        return false;
    if (prepare) {
        pinMode(chipSelect, OUTPUT);
        digitalWrite(chipSelect, HIGH);
        pinMode(reset, OUTPUT);
        digitalWrite(reset, LOW);
        delay(2);
        digitalWrite(reset, HIGH);
        delay(50);
    }
    spi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(chipSelect, LOW);
    spi.transfer(0x80 | (0x37 << 1));  // MFRC522 VersionReg, read-only.
    const auto version = spi.transfer(0);
    digitalWrite(chipSelect, HIGH);
    spi.endTransaction();
    return version == 0x90 || version == 0x91 || version == 0x92 || version == 0x88 ||
           version == 0x89 || version == 0xB2;
}

RoomIdentityReading RoomHardwareProbe::identity(uint8_t pin) {
    if (!HardwareCheck::identityPin(pin))
        return {};
    pinMode(pin, ANALOG);  // External pull-up only; internal pulls distort the divider.
    analogReadResolution(12);
    analogSetPinAttenuation(pin, ADC_11db);
    delay(2);  // Settle the divider and its 100 nF bypass capacitor.
    analogReadMilliVolts(pin);  // Discard the first conversion after configuring the ADC.
    unsigned samples[32];
    for (auto& sample : samples) {
        sample = analogReadMilliVolts(pin);
        delayMicroseconds(250);
    }
    std::sort(samples, samples + 32);
    unsigned total = 0;
    for (unsigned i = 4; i < 28; ++i)
        total += samples[i];
    RoomIdentityReading result;
    result.millivolts = total / 24;
    result.spreadMillivolts = samples[27] - samples[4];
    return result;
}
