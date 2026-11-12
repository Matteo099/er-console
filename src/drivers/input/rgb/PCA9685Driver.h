#pragma once

#include <Adafruit_PWMServoDriver.h>

#include "IRgbLedDriver.h"

class PCA9685Driver : public IRgbLedDriver {
  public:
    PCA9685Driver(TwoWire& wire, uint8_t address = 0x40);
    bool init() override;
    void shutdown() override;
    bool available() const override;
    void setRGB(uint8_t channel, const LedColor& color) override;
    void setBrightness(uint8_t channel, uint8_t brightness) override;

  private:
    TwoWire* wireBus = nullptr;
    Adafruit_PWMServoDriver pwm;
    uint8_t address;
    bool initialized = false;
    uint8_t brightness[16];
    void writeChannel(uint8_t channel, uint16_t value);
};