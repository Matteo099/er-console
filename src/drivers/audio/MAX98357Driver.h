#pragma once

#include "AudioDriver.h"
#include <driver/i2s.h>

class MAX98357Driver : public AudioDriver
{
public:
    MAX98357Driver(int bclk, int lrck, int dout);
    bool init() override;
    void playTone(uint16_t freq, uint16_t ms) override;
    void stop() override;
    void playWav(IFile* file) override;

private:
    int pinBCLK;
    int pinLRCK;
    int pinDOUT;

    void applyVolume(int16_t* buffer, size_t samples);
};
