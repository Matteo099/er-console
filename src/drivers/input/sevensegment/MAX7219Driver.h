#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "ISevenSegmentDriver.h"

class MAX7219Driver : public ISevenSegmentDriver {
  public:
    MAX7219Driver(SPIClass& spi, uint8_t csPin, uint8_t digits = 8);

    bool init() override;
    void shutdown() override;
    bool available() const override;
    uint8_t digits() const override;
    void clear() override;
    void setDigit(uint8_t digit, char value, bool dot = false) override;
    void setBrightness(uint8_t value) override;

  private:
    void writeRegister(uint8_t reg, uint8_t data);
    SPIClass* spiBus = nullptr;
    uint8_t csPin;
    uint8_t digitCount;
    bool initialized = false;
    uint8_t brightness = 8;
};