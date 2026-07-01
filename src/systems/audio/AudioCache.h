#pragma once

#include <array>
#include <cstdint>

#include "systems/storage/StorageSystem.h"
#include "systems/audio/MemoryAudioSource.h"
#include "systems/audio/AudioCacheEntry.h"

#ifndef AUDIO_CACHE_SIZE
#define AUDIO_CACHE_SIZE 8
#endif

class AudioCache
{
public:
    AudioCache() = default;
    ~AudioCache();

    /**
     * Initializes the cache.
     */
    bool init(StorageSystem *storage);

    /**
     * Returns a cached audio source.
     *
     * If the file is not cached, it is loaded automatically.
     *
     * Returns nullptr on failure.
     */
    MemoryAudioSource *get(const char *path);

    /**
     * Prevents a cache entry from being evicted.
     */
    void retain(MemoryAudioSource *source);

    /**
     * Releases a retained entry.
     */
    void release(MemoryAudioSource *source);

    /**
     * Removes every cached sound.
     *
     * Entries currently retained are left untouched.
     */
    void clear();

    /**
     * Number of cached entries.
     */
    size_t size() const;

private:
    AudioCacheEntry *find(const char *path);

    AudioCacheEntry *load(const char *path);

    /**
     * Finds a free cache slot.
     *
     * If none is available, returns the least recently
     * used entry whose refCount == 0.
     */
    AudioCacheEntry *allocateSlot();

    /**
     * Frees all memory owned by an entry.
     */
    void releaseEntry(AudioCacheEntry &entry, bool force = false);

private:
    StorageSystem *storage = nullptr;
    uint32_t counter = 0;
    std::array<AudioCacheEntry, AUDIO_CACHE_SIZE> entries;
};