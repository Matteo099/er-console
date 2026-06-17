#include "NativeGpioDriver.h"

#include <Arduino.h>

bool NativeGpioDriver::init() { return true; }

bool NativeGpioDriver::ownsPin(uint16_t pin) const { return pin < 40; }

void NativeGpioDriver::pinMode(uint16_t pin, uint8_t mode) { ::pinMode(pin, mode); }

void NativeGpioDriver::digitalWrite(uint16_t pin, bool state) { ::digitalWrite(pin, state); }

bool NativeGpioDriver::digitalRead(uint16_t pin) { return ::digitalRead(pin); }