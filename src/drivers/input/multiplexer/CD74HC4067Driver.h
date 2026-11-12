#pragma once

#include <Arduino.h>

#include "IMultiplexer.h"

class CD74HC4067Driver : public IMultiplexer {
  public:
    CD74HC4067Driver(uint8_t signalPin, uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3,
                     int8_t enablePin = -1);

    bool init() override;

    void shutdown() override;

    uint8_t channelCount() const override;

    bool selectChannel(uint8_t channel) override;

    int analogRead(uint8_t channel) override;

    bool digitalRead(uint8_t channel) override;

    void digitalWrite(uint8_t channel, bool value) override;

  private:
    void setAddress(uint8_t channel);

  private:
    uint8_t signalPin;

    uint8_t s0Pin;
    uint8_t s1Pin;
    uint8_t s2Pin;
    uint8_t s3Pin;

    int8_t enablePin;

    uint8_t currentChannel = 255;

    bool initialized = false;
};