#pragma once

#include <cstddef>
#include <cstdint>
#include "AudioEvent.h"
#include "MemoryAudioSource.h"

#define AUDIO_CACHE_SIZE 4

struct AudioCacheEntry
{
    char path[AudioEvent::MAX_PATH_LENGTH];
    uint8_t *data = nullptr;
    size_t size = 0;
    uint32_t lastUsed = 0;

    MemoryAudioSource file;
};

