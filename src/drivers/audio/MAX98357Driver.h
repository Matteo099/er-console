#pragma once

#include "AudioDriver.h"
#include <driver/i2s.h>

class MAX98357Driver : public AudioDriver {
  public:
    MAX98357Driver(int bclk, int lrck, int dout);
    bool init() override;
    void playTone(uint16_t freq, uint16_t ms) override;
    bool startWav(IFile* file) override;
    void update() override;
    void stop() override;

  private:
    int pinBCLK;
    int pinLRCK;
    int pinDOUT;

    IFile* currentFile = nullptr;

    static constexpr size_t BUFFER_SIZE = 1024;
    uint8_t buffer[BUFFER_SIZE];

    void applyVolume(int16_t* buffer, size_t samples);
};
