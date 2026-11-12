#include "ESP32PwmRgbDriver.h"

ESP32PwmRgbDriver::ESP32PwmRgbDriver(uint8_t rPin, uint8_t gPin, uint8_t bPin)
    : rPin(rPin), gPin(gPin), bPin(bPin) {
}

bool ESP32PwmRgbDriver::init() {
    constexpr uint8_t RED_CHANNEL = 0;
    constexpr uint8_t GREEN_CHANNEL = 1;
    constexpr uint8_t BLUE_CHANNEL = 2;
    constexpr uint32_t FREQUENCY = 5000;
    constexpr uint8_t RESOLUTION = 8;
    ledcSetup(RED_CHANNEL, FREQUENCY, RESOLUTION);
    ledcSetup(GREEN_CHANNEL, FREQUENCY, RESOLUTION);
    ledcSetup(BLUE_CHANNEL, FREQUENCY, RESOLUTION);
    ledcAttachPin(rPin, RED_CHANNEL);
    ledcAttachPin(gPin, GREEN_CHANNEL);
    ledcAttachPin(bPin, BLUE_CHANNEL);
    initialized = true;
    return true;
}

void ESP32PwmRgbDriver::shutdown() {
    if (!initialized)
        return;

    // Turn LED off before releasing PWM
    ledcWrite(0, 0);
    ledcWrite(1, 0);
    ledcWrite(2, 0);

    // Stop PWM channels
    ledcDetachPin(rPin);
    ledcDetachPin(gPin);
    ledcDetachPin(bPin);

    // Release GPIOs
    pinMode(rPin, INPUT);
    pinMode(gPin, INPUT);
    pinMode(bPin, INPUT);

    // Reset state
    brightness = 255;
    initialized = false;
}

bool ESP32PwmRgbDriver::available() const {
    return initialized;
}

void ESP32PwmRgbDriver::writePWM(uint8_t channel, uint8_t value) {
    uint16_t pwm = value * brightness / 255;
    ledcWrite(channel, pwm);
}

void ESP32PwmRgbDriver::setRGB(uint8_t, const LedColor& color) {
    writePWM(0, color.r);
    writePWM(1, color.g);
    writePWM(2, color.b);
}

void ESP32PwmRgbDriver::setBrightness(uint8_t, uint8_t value) {
    brightness = value;
}