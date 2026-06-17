#include "MCP23017Driver.h"

bool MCP23017Driver::init()
{
    initialized = mcp.begin_I2C(address);
    return initialized;
}

bool MCP23017Driver::ownsPin(uint16_t pin) const { return pin >= basePin && pin < basePin + 16; }

uint16_t MCP23017Driver::toLocal(uint16_t pin) const { return pin - basePin; }

void MCP23017Driver::pinMode(uint16_t pin, uint8_t mode) { mcp.pinMode(toLocal(pin), mode); }

void MCP23017Driver::digitalWrite(uint16_t pin, bool state) { mcp.digitalWrite(toLocal(pin), state); }

bool MCP23017Driver::digitalRead(uint16_t pin) { return mcp.digitalRead(toLocal(pin)); }