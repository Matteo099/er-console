#pragma once

#include <stdint.h>

class IMultiplexer {
  public:
    virtual ~IMultiplexer() = default;

    virtual bool init() = 0;

    virtual void shutdown() = 0;

    virtual uint8_t channelCount() const = 0;

    virtual bool selectChannel(uint8_t channel) = 0;

    virtual int analogRead(uint8_t channel) = 0;

    virtual bool digitalRead(uint8_t channel) = 0;

    virtual void digitalWrite(uint8_t channel, bool value) = 0;
};