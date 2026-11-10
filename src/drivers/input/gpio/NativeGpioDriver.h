#pragma once

#include <unordered_set>
#include "IGpioProvider.h"

class NativeGpioDriver : public IGpioProvider
{
public:
    bool init() override;
    void shutdown() override;
    bool ownsPin(uint16_t pin) const override;
    void pinMode(uint16_t pin, uint8_t mode) override;
    void digitalWrite(uint16_t pin, bool state) override;
    bool digitalRead(uint16_t pin) override;
private:
    std::unordered_set<uint16_t> configuredPins;
};