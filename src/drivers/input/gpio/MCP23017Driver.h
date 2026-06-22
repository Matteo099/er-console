#pragma once

#include <Adafruit_MCP23X17.h>

#include "IGpioProvider.h"

class MCP23017Driver : public IGpioProvider
{
public:
    MCP23017Driver(uint16_t basePin, uint8_t address);
    bool init() override;
    void shutdown() override;
    bool ownsPin(uint16_t pin) const override;
    void pinMode(uint16_t pin, uint8_t mode) override;
    void digitalWrite(uint16_t pin, bool state) override;
    bool digitalRead(uint16_t pin) override;

private:
    uint16_t toLocal(uint16_t pin) const;

private:
    uint16_t basePin;
    uint8_t address;
    bool initialized = false;
    Adafruit_MCP23X17 mcp;
};