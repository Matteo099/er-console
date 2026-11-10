#pragma once

#include "IChannelProvider.h"
#include <stdint.h>
#include "../multiplexer/CD74HC4067Driver.h"
class MuxChannelProvider : public IChannelProvider
{
public:
    MuxChannelProvider(CD74HC4067Driver &mux, uint16_t base) : mux(mux), base(base) {}
    
    bool init() override { return mux.init(); }

    void shutdown() override { mux.shutdown(); }

    bool owns(uint16_t c) const override { return c >= base && c < base + 16; }

    void pinMode(uint16_t c, uint8_t mode) override { /* Mux channels can't set pinMode, so we ignore this */ }

    int analogRead(uint16_t c) { return mux.analogRead(c - base); }

    void analogWrite(uint16_t c, int value) { /* Mux channels don't support analogWrite */ } 
    
    bool digitalRead(uint16_t c) { return mux.digitalRead(c - base); }

    void digitalWrite(uint16_t c, bool value) { mux.digitalWrite(c - base, value); }


private:
    CD74HC4067Driver &mux;
    uint16_t base;
};