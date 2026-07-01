#pragma once

#include <cstdint>

#include "MemoryAudioSource.h"

#ifndef AUDIO_MAX_PATH
#define AUDIO_MAX_PATH 128
#endif

struct AudioCacheEntry
{
    char path[AUDIO_MAX_PATH]{};

    uint8_t *data = nullptr;

    size_t size = 0;

    uint32_t accessCounter = 0;

    uint16_t refCount = 0;

    MemoryAudioSource source;

    bool valid() const
    {
        return data != nullptr;
    }
};