#include "MCP23017Driver.h"

#include "drivers/bus/I2CBusLock.h"

MCP23017Driver::MCP23017Driver(TwoWire& wire, uint16_t basePin, uint8_t address)
    : wireBus(&wire), basePin(basePin), address(address) {
}

bool MCP23017Driver::init() {
    I2CBusLock::Guard guard;
    initialized = mcp.begin_I2C(address, wireBus);
    return initialized;
}

void MCP23017Driver::shutdown() {
    if (!initialized)
        return;

    I2CBusLock::Guard guard;
    for (uint8_t pin = 0; pin < 16; pin++) {
        mcp.digitalWrite(pin, LOW);
        mcp.pinMode(pin, INPUT);
    }
}

bool MCP23017Driver::ownsPin(uint16_t pin) const {
    return pin >= basePin && pin < basePin + 16;
}

uint16_t MCP23017Driver::toLocal(uint16_t pin) const {
    return pin - basePin;
}

void MCP23017Driver::pinMode(uint16_t pin, uint8_t mode) {
    I2CBusLock::Guard guard;
    mcp.pinMode(toLocal(pin), mode);
}

void MCP23017Driver::digitalWrite(uint16_t pin, bool state) {
    I2CBusLock::Guard guard;
    mcp.digitalWrite(toLocal(pin), state);
}

bool MCP23017Driver::digitalRead(uint16_t pin) {
    I2CBusLock::Guard guard;
    return mcp.digitalRead(toLocal(pin));
}