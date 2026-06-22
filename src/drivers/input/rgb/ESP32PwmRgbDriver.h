#pragma once

#include <Arduino.h>

#include "IRgbLedDriver.h"

class ESP32PwmRgbDriver : public IRgbLedDriver
{
public:
    ESP32PwmRgbDriver(uint8_t rPin, uint8_t gPin, uint8_t bPin);
    bool init() override;
    void shutdown() override;
    bool available() const override;
    void setRGB(uint8_t channel, const LedColor &color) override;
    void setBrightness(uint8_t channel, uint8_t brightness) override;

private:
    uint8_t rPin;
    uint8_t gPin;
    uint8_t bPin;
    uint8_t brightness = 255;
    bool initialized = false;
    void writePWM(uint8_t channel, uint8_t value);
};