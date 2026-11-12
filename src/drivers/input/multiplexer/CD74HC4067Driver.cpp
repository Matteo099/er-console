#include "CD74HC4067Driver.h"

CD74HC4067Driver::CD74HC4067Driver(uint8_t signalPin, uint8_t s0, uint8_t s1, uint8_t s2,
                                   uint8_t s3, int8_t enablePin)
    : signalPin(signalPin), s0Pin(s0), s1Pin(s1), s2Pin(s2), s3Pin(s3), enablePin(enablePin) {
}

bool CD74HC4067Driver::init() {
    pinMode(s0Pin, OUTPUT);
    pinMode(s1Pin, OUTPUT);
    pinMode(s2Pin, OUTPUT);
    pinMode(s3Pin, OUTPUT);

    if (enablePin >= 0) {
        pinMode(enablePin, OUTPUT);
        digitalWrite(enablePin, LOW);
    }

    initialized = true;

    return true;
}

void CD74HC4067Driver::shutdown() {
    if (!initialized)
        return;

    // Disable the mux first
    if (enablePin >= 0) {
        digitalWrite(enablePin, HIGH);
        pinMode(enablePin, INPUT);
    }

    // Clear address lines
    digitalWrite(s0Pin, LOW);
    digitalWrite(s1Pin, LOW);
    digitalWrite(s2Pin, LOW);
    digitalWrite(s3Pin, LOW);

    // Release GPIOs
    pinMode(s0Pin, INPUT);
    pinMode(s1Pin, INPUT);
    pinMode(s2Pin, INPUT);
    pinMode(s3Pin, INPUT);

    currentChannel = 0;
    initialized = false;
}

uint8_t CD74HC4067Driver::channelCount() const {
    return 16;
}

void CD74HC4067Driver::setAddress(uint8_t channel) {
    digitalWrite(s0Pin, channel & 0x01);
    digitalWrite(s1Pin, channel & 0x02);
    digitalWrite(s2Pin, channel & 0x04);
    digitalWrite(s3Pin, channel & 0x08);

    currentChannel = channel;
}

bool CD74HC4067Driver::selectChannel(uint8_t channel) {
    if (channel >= 16)
        return false;

    setAddress(channel);

    return true;
}

int CD74HC4067Driver::analogRead(uint8_t channel) {
    if (channel >= 16)
        return 0;

    setAddress(channel);

    return ::analogRead(signalPin);
}

bool CD74HC4067Driver::digitalRead(uint8_t channel) {
    if (channel >= 16)
        return false;

    setAddress(channel);

    return ::digitalRead(signalPin);
}

void CD74HC4067Driver::digitalWrite(uint8_t channel, bool value) {
    if (channel >= 16)
        return;

    setAddress(channel);

    ::digitalWrite(signalPin, value);
}