#pragma once

#include "IChannelProvider.h"

#include <stdint.h>
#include "Arduino.h"

class NativeChannelProvider : public IChannelProvider
{

public:
    bool init() override
    {
        return true;
    }
    
    bool owns(uint16_t c) const override
    {
        return c < 40;
    }

    void pinMode(
        uint16_t c,
        uint8_t mode)
    {
        ::pinMode(c, mode);
    }

    void digitalWrite(
        uint16_t c,
        bool state)
    {
        ::digitalWrite(c, state);
    }

    bool digitalRead(
        uint16_t c)
    {
        return ::digitalRead(c);
    }

    int analogRead(
        uint16_t c)
    {
        return ::analogRead(c);
    }
};