#include "AudioCache.h"

#include <cstring>
#include <new>

#include "log/Logger.h"

AudioCache::~AudioCache()
{
    clear();
}

bool AudioCache::init(StorageSystem *storage)
{
    if (!storage)
        return false;

    this->storage = storage;
    clear();
    return true;
}

void AudioCache::clear()
{
    for (auto &entry : entries)
    {
        releaseEntry(entry, true);
    }
    counter = 0;
}

size_t AudioCache::size() const
{
    size_t count = 0;

    for (const auto &entry : entries)
    {
        if (entry.valid())
            ++count;
    }

    return count;
}

MemoryAudioSource *AudioCache::get(const char *path)
{
    if (!storage || !path || !*path)
        return nullptr;

    AudioCacheEntry *entry = find(path);

    if (!entry)
    {
        entry = load(path);
        if (!entry)
            return nullptr;
    }

    entry->accessCounter = ++counter;
    return &entry->source;
}

AudioCacheEntry *AudioCache::find(
    const char *path)
{
    for (auto &entry : entries)
    {
        if (!entry.valid())
            continue;

        if (strcmp(entry.path, path) == 0)
        {
            return &entry;
        }
    }

    return nullptr;
}

AudioCacheEntry *AudioCache::allocateSlot()
{
    //
    // First look for an unused slot.
    //
    for (auto &entry : entries)
    {
        if (!entry.valid())
            return &entry;
    }

    //
    // LRU eviction.
    // Entries currently in use cannot
    // be evicted.
    //
    AudioCacheEntry *candidate = nullptr;
    uint32_t oldest = UINT32_MAX;
    for (auto &entry : entries)
    {
        if (entry.refCount != 0)
            continue;

        if (entry.accessCounter < oldest)
        {
            oldest = entry.accessCounter;
            candidate = &entry;
        }
    }

    if (!candidate)
    {
        Log::warn(Log::AUDIO_TAG, "Audio cache full (all entries locked)");
        return nullptr;
    }

    releaseEntry(*candidate);

    return candidate;
}

AudioCacheEntry *AudioCache::load(const char *path)
{
    if (!path || !storage)
        return nullptr;

    AudioCacheEntry *entry = allocateSlot();

    if (!entry)
        return nullptr;

    // Make sure the slot is clean
    releaseEntry(*entry, true);

    IFile *file = storage->open(path);

    if (!file)
        return nullptr;

    //
    // Skip WAV header
    //
    if (!file->seek(44))
    {
        file->close();
        delete file;
        return nullptr;
    }

    //
    // Determine PCM size
    //
    size_t pcmSize = 0;
    uint8_t temp[1024];
    while (file->available())
    {
        int read = file->read(temp, sizeof(temp));
        if (read <= 0)
            break;
        pcmSize += read;
    }

    if (pcmSize == 0)
    {
        file->close();
        delete file;
        return nullptr;
    }

    uint8_t *buffer = new (std::nothrow) uint8_t[pcmSize];
    if (!buffer)
    {
        file->close();
        delete file;
        return nullptr;
    }

    //
    // Read PCM data
    //
    if (!file->seek(44))
    {
        delete[] buffer;
        file->close();
        delete file;
        return nullptr;
    }

    size_t offset = 0;

    while (offset < pcmSize && file->available())
    {
        int read = file->read(buffer + offset, pcmSize - offset);
        if (read <= 0)
            break;
        offset += read;
    }

    file->close();
    delete file;
    if (offset != pcmSize)
    {
        delete[] buffer;
        releaseEntry(*entry, true);
        return nullptr;
    }

    //
    // Fill cache entry
    //
    strncpy(entry->path, path, AUDIO_MAX_PATH - 1);
    entry->path[AUDIO_MAX_PATH - 1] = '\0';
    entry->data = buffer;
    entry->size = pcmSize;
    entry->accessCounter = ++counter;
    entry->refCount = 0;

    //
    // IMPORTANT:
    // assign expects bytes, not samples
    //
    entry->source.assign(entry->data, entry->size);
    Log::debug(Log::AUDIO_TAG, "Cached %s (%u bytes)", path, static_cast<uint32_t>(pcmSize));
    return entry;
}

void AudioCache::retain(MemoryAudioSource *source)
{
    if (!source)
        return;

    for (auto &entry : entries)
    {
        if (&entry.source != source)
            continue;

        ++entry.refCount;
        return;
    }
}

void AudioCache::release(MemoryAudioSource *source)
{
    if (!source)
        return;

    for (auto &entry : entries)
    {
        if (&entry.source != source)
            continue;

        if (entry.refCount)
            --entry.refCount;

        return;
    }
}

void AudioCache::releaseEntry(AudioCacheEntry &entry, bool force)
{
    if (!force && entry.refCount != 0)
    {
        return;
    }

    delete[] entry.data;
    entry.data = nullptr;
    entry.size = 0;
    entry.accessCounter = 0;
    entry.refCount = 0;
    entry.path[0] = '\0';
    entry.source.assign(nullptr, 0);
}