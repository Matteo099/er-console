#pragma once

#include <Arduino.h>
#include <driver/i2s.h>

class MAX98357Driver
{
public:
    MAX98357Driver(int bclk, int lrck, int dout);
    bool init(uint32_t sampleRate = 16000);
    bool write(const int16_t *samples, size_t sampleCount);
    void clear();
    void setEnabled(bool enabled);

private:
    int pinBCLK;
    int pinLRCK;
    int pinDOUT;

    bool initialized = false;
};