#include "RgbLedSystem.h"

void RgbLedSystem::registerDriver(IRgbLedDriver* driver) {
    drivers.push_back(driver);
}

bool RgbLedSystem::init() {
    for (auto d : drivers)
        if (!d->init())
            return false;
    return true;
}

void RgbLedSystem::update() {
}

void RgbLedSystem::setColor(uint8_t led, LedColor color) {
    if (led >= drivers.size())
        return;
    drivers[led]->setRGB(0, color);
}

void RgbLedSystem::setBrightness(uint8_t led, uint8_t value) {
    if (led >= drivers.size())
        return;
    drivers[led]->setBrightness(0, value);
}

void RgbLedSystem::shutdown() {
    for (auto d : drivers) {
        d->shutdown();
    }
}