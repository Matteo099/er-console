#pragma once

#include <stdint.h>

class IGpioProvider {
  public:
    virtual ~IGpioProvider() = default;
    virtual bool init() = 0;
    virtual void shutdown() = 0;
    virtual bool ownsPin(uint16_t pin) const = 0;
    virtual void pinMode(uint16_t pin, uint8_t mode) = 0;
    virtual void digitalWrite(uint16_t pin, bool state) = 0;
    virtual bool digitalRead(uint16_t pin) = 0;
};