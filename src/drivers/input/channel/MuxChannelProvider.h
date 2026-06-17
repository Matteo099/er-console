#pragma once

#include "IChannelProvider.h"
#include <stdint.h>
#include "../multiplexer/CD74HC4067Driver.h"

class MuxChannelProvider : public IChannelProvider
{

public:
    MuxChannelProvider(
        CD74HC4067Driver &mux,
        uint16_t base)
        : mux(mux),
          base(base)
    {
    }

    bool owns(uint16_t c) const override
    {
        return c >= base &&
               c < base + 16;
    }

    int analogRead(
        uint16_t c)
    {
        return mux.analogRead(
            c - base);
    }

    bool digitalRead(
        uint16_t c)
    {
        return mux.digitalRead(
            c - base);
    }

    void digitalWrite(
        uint16_t c,
        bool value)
    {
        mux.digitalWrite(
            c - base,
            value);
    }

private:
    CD74HC4067Driver &mux;

    uint16_t base;
};