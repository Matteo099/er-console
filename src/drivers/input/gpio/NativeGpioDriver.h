#pragma once

#include "IGpioProvider.h"

class NativeGpioDriver : public IGpioProvider
{
public:
    bool init() override;
    bool ownsPin(uint16_t pin) const override;
    void pinMode(uint16_t pin, uint8_t mode) override;
    void digitalWrite(uint16_t pin, bool state) override;
    bool digitalRead(uint16_t pin) override;
};