#pragma once

#include "IChannelProvider.h"
#include <stdint.h>
#include "../gpio/MCP23017Driver.h"
class MCPChannelProvider : public IChannelProvider
{
public:
    MCPChannelProvider(MCP23017Driver &driver, uint16_t base) : driver(driver), base(base) {}
    
    bool init() override { return driver.init(); }
    
    bool owns(uint16_t c) const override { return c >= base && c < base + 16; }
    
    void pinMode(uint16_t c, uint8_t mode) override { driver.pinMode(c - base, mode); }
    
    void digitalWrite(uint16_t c, bool value) { driver.digitalWrite(c - base, value); }
    
    bool digitalRead(uint16_t c) { return driver.digitalRead(c - base); }
    
    int analogRead(uint16_t c) override { return -1; }

    void analogWrite(uint16_t c, int value) override {  }

private:
    MCP23017Driver &driver;
    uint16_t base;
};