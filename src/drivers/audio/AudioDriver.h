#pragma once

#include <Arduino.h>

class AudioDriver
{
public:
    virtual ~AudioDriver() = default;

    virtual bool init(uint32_t sampleRate = 16000) = 0;
    virtual bool write(const int16_t *samples, size_t sampleCount) = 0;
    virtual void clear() = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual bool enabled() const = 0;
};
