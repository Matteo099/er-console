#pragma once

#include "RGBColor.h"

class IRgbLedDriver
{
public:
    virtual ~IRgbLedDriver() = default;
    virtual bool init() = 0;
    virtual bool available() const = 0;
    virtual void setRGB(uint8_t channel, const RGBColor &color) = 0;
    virtual void setBrightness(uint8_t channel, uint8_t brightness) = 0;
};