#include "RoomHardwareProbe.h"
#include "drivers/bus/I2CBusManager.h"
#include "drivers/bus/I2CBusLock.h"
#include "drivers/bus/SPIBusManager.h"
#include "drivers/bus/SPIBusLock.h"
#include <Arduino.h>
#include <OneWire.h>

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

std::string RoomHardwareProbe::identity(uint8_t pin) {
    if (!HardwareCheck::roomPin(pin))
        return {};
    OneWire wire(pin);
    uint8_t address[8], extra[8];
    if (!wire.search(address) || OneWire::crc8(address, 7) != address[7] || address[0] != 0x01 ||
        wire.search(extra))
        return {};
    char rom[17]{};
    for (unsigned i = 0; i < 8; ++i)
        snprintf(rom + 2 * i, 3, "%02X", address[i]);
    return rom;
}
