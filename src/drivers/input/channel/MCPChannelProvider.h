#pragma once

#include "IChannelProvider.h"
#include <stdint.h>
#include "../gpio/MCP23017Driver.h"

class MCPChannelProvider : public IChannelProvider
{

public:
    MCPChannelProvider(
        MCP23017Driver &driver,
        uint16_t base)
        : driver(driver),
          base(base)
    {
    }

    bool owns(uint16_t c) const override
    {
        return c >= base &&
               c < base + 16;
    }

    void digitalWrite(
        uint16_t c,
        bool value)
    {
        driver.digitalWrite(
            c - base,
            value);
    }

    bool digitalRead(
        uint16_t c)
    {
        return driver.digitalRead(
            c - base);
    }

private:
    MCP23017Driver &driver;

    uint16_t base;
};