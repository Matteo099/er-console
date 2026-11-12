#include "PCA9685Driver.h"

#include "drivers/bus/I2CBusLock.h"

PCA9685Driver::PCA9685Driver(TwoWire& wire, uint8_t address)
    : wireBus(&wire), pwm(address, wire), address(address) {
}

bool PCA9685Driver::init() {
    I2CBusLock::Guard guard;

    if (!pwm.begin())
        return false;
    pwm.setPWMFreq(1000);
    for (int i = 0; i < 16; i++)
        brightness[i] = 255;
    initialized = true;
    return true;
}

void PCA9685Driver::shutdown() {
    if (!initialized)
        return;

    I2CBusLock::Guard guard;

    // Turn off all PWM outputs
    for (uint8_t channel = 0; channel < 16; channel++) {
        pwm.setPWM(channel, 0, 0);
        brightness[channel] = 255;
    }

    // Put PCA9685 into sleep mode
    // Stops oscillator and PWM generation
    pwm.sleep();

    initialized = false;
}

bool PCA9685Driver::available() const {
    return initialized;
}

void PCA9685Driver::writeChannel(uint8_t channel, uint16_t value) {
    I2CBusLock::Guard guard;
    pwm.setPWM(channel, 0, value);
}

void PCA9685Driver::setRGB(uint8_t channel, const LedColor& c) {
    uint16_t r = c.r * brightness[channel] / 255;
    uint16_t g = c.g * brightness[channel] / 255;
    uint16_t b = c.b * brightness[channel] / 255;
    uint8_t base = channel * 3;
    writeChannel(base, r * 16);
    writeChannel(base + 1, g * 16);
    writeChannel(base + 2, b * 16);
}

void PCA9685Driver::setBrightness(uint8_t channel, uint8_t value) {
    brightness[channel] = value;
}