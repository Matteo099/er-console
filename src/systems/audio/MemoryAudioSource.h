#pragma once

#include "AudioSource.h"

class MemoryAudioSource : public AudioSource
{
public:
    MemoryAudioSource() = default;

    /**
     * Assigns a WAV file stored entirely in memory.
     *
     * The source assumes:
     * - 44-byte WAV header
     * - PCM
     * - 16-bit
     * - Mono
     */
    void assign(
        const uint8_t *data,
        size_t size);

    size_t read(
        int16_t *dst,
        size_t sampleCount) override;

    void reset() override;

    bool eof() const override;

    size_t position() const override;

    size_t length() const override;

    uint32_t sampleRate() const override;

    uint8_t channels() const override;

    uint8_t bitsPerSample() const override;

private:
    static constexpr size_t WAV_HEADER_SIZE = 44;

    const int16_t *samples = nullptr;

    size_t sampleCount = 0;
    size_t currentSample = 0;
};