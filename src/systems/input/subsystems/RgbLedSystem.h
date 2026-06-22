#pragma once

#include <vector>

#include "ISubsystem.h"
#include "drivers/input/rgb/IRgbLedDriver.h"

class RgbLedSystem : public ISubsystem
{
public:
    void registerDriver(IRgbLedDriver *driver);
    bool init() override;
    void update() override;
    void shutdown() override;
    void setColor(uint8_t led, LedColor color);
    void setBrightness(uint8_t led, uint8_t value);

private:
    std::vector<IRgbLedDriver *> drivers;
};