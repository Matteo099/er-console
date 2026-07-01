#pragma once

#include "AudioDriver.h"
#include <driver/i2s.h>

class MAX98357Driver : public AudioDriver
{
public:
    MAX98357Driver(int bclk, int lrck, int dout);
    bool init(uint32_t sampleRate = 16000) override;
    bool write(const int16_t *samples, size_t sampleCount) override;
    void clear() override;
    void setEnabled(bool enabled) override;
    bool enabled() const override
    {
        return amplifierEnabled;
    }

private:
    int pinBCLK;
    int pinLRCK;
    int pinDOUT;

    bool initialized = false;
    bool amplifierEnabled = false;
};