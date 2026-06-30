#include "MemoryAudioSource.h"

#include <cstring>

void MemoryAudioSource::assign(
    const uint8_t *data,
    size_t size)
{
    samples = nullptr;
    sampleCount = 0;
    currentSample = 0;

    if (!data)
        return;

    if (size <= WAV_HEADER_SIZE)
        return;

    samples = reinterpret_cast<const int16_t *>(
        data + WAV_HEADER_SIZE);

    sampleCount =
        (size - WAV_HEADER_SIZE) / sizeof(int16_t);
}

size_t MemoryAudioSource::read(
    int16_t *dst,
    size_t requestedSamples)
{
    if (!samples || !dst)
        return 0;

    const size_t remaining =
        sampleCount - currentSample;

    const size_t count =
        remaining < requestedSamples
            ? remaining
            : requestedSamples;

    if (count == 0)
        return 0;

    memcpy(
        dst,
        samples + currentSample,
        count * sizeof(int16_t));

    currentSample += count;

    return count;
}

void MemoryAudioSource::reset()
{
    currentSample = 0;
}

bool MemoryAudioSource::eof() const
{
    return currentSample >= sampleCount;
}

size_t MemoryAudioSource::position() const
{
    return currentSample;
}

size_t MemoryAudioSource::length() const
{
    return sampleCount;
}

uint32_t MemoryAudioSource::sampleRate() const
{
    return 16000;
}

uint8_t MemoryAudioSource::channels() const
{
    return 1;
}

uint8_t MemoryAudioSource::bitsPerSample() const
{
    return 16;
}