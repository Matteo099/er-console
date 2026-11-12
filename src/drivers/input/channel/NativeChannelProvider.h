#pragma once

#include "IChannelProvider.h"

#include <stdint.h>
#include <unordered_set>
#include "Arduino.h"
#include "config/BoardConfig.h"

class NativeChannelProvider : public IChannelProvider {
  public:
    bool init() override {
        return true;
    }

    void shutdown() override {
        for (uint16_t pin : configuredPins) {
            gpio_reset_pin(static_cast<gpio_num_t>(pin));
        }

        configuredPins.clear();
    }

    bool owns(uint16_t c) const override {
        return c >= GPIO_MIN_PIN && c <= GPIO_MAX_PIN;
    }

    void pinMode(uint16_t c, uint8_t mode) {
        ::pinMode(c, mode);
        configuredPins.insert(c);
    }

    void digitalWrite(uint16_t c, bool state) {
        ::digitalWrite(c, state);
    }

    bool digitalRead(uint16_t c) {
        return ::digitalRead(c);
    }

    int analogRead(uint16_t c) {
        return ::analogRead(c);
    }

    void analogWrite(uint16_t c, int value) {
        ::analogWrite(c, value);
    }

  private:
    std::unordered_set<uint16_t> configuredPins;
};