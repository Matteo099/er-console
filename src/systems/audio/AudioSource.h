#pragma once

#include <cstddef>
#include <cstdint>

class AudioSource
{
public:
    virtual ~AudioSource() = default;

    /**
     * Reads up to sampleCount PCM samples.
     *
     * @param dst Destination buffer.
     * @param sampleCount Maximum number of samples to read.
     * @return Number of samples actually read.
     */
    virtual size_t read(
        int16_t *dst,
        size_t sampleCount) = 0;

    /**
     * Resets playback to the beginning.
     */
    virtual void reset() = 0;

    /**
     * Returns true when the source has no more samples.
     */
    virtual bool eof() const = 0;

    /**
     * Current playback position in samples.
     */
    virtual size_t position() const = 0;

    /**
     * Total number of samples.
     * Returns 0 if unknown (e.g. streaming source).
     */
    virtual size_t length() const = 0;

    /**
     * Sample rate of this source.
     * (Usually 16000 Hz in your project.)
     */
    virtual uint32_t sampleRate() const = 0;

    /**
     * Number of channels.
     * Currently always 1 (mono).
     */
    virtual uint8_t channels() const = 0;

    /**
     * Bits per sample.
     * Currently always 16.
     */
    virtual uint8_t bitsPerSample() const = 0;
};