#include "RgbLedSystem.h"

void RgbLedSystem::registerDriver(IRgbLedDriver *driver) { drivers.push_back(driver); }

bool RgbLedSystem::init()
{
    for (auto d : drivers)
        d->init();
    return true;
}

void RgbLedSystem::update() {}

void RgbLedSystem::setColor(uint8_t led, RGBColor color)
{
    if (led >= drivers.size())
        return;
    drivers[led]->setRGB(0, color);
}

void RgbLedSystem::setBrightness(uint8_t led, uint8_t value)
{
    if (led >= drivers.size())
        return;
    drivers[led]->setBrightness(0, value);
}