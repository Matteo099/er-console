#pragma once

#include <cstdint>
#include "drivers/storage/IFile.h"

class AudioDriver {
  protected:
    uint8_t currentVolume = 50;
    bool playing = false;

  public:
    virtual ~AudioDriver() = default;

    virtual bool init() = 0;
    virtual void stop() = 0;
    virtual void update() = 0;

    virtual void playTone(uint16_t freq, uint16_t ms) = 0;
    virtual bool startWav(IFile* file) = 0;

    bool isPlaying() const {
        return playing;
    };

    void setVolume(uint8_t volume) {
        currentVolume = volume;
    };
};
