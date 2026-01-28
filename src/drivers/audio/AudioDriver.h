#pragma once

#include <cstdint>
#include "drivers/storage/IFile.h"

class AudioDriver
{
protected:
    uint8_t currentVolume = 50;

public:
    virtual ~AudioDriver() = default;

    virtual bool init() = 0;
    virtual void stop() = 0;

    virtual void playTone(uint16_t freq, uint16_t ms) = 0;
    virtual void playWav(IFile* file) = 0;

    void setVolume(uint8_t volume)
    {
        currentVolume = volume;
    };
};
