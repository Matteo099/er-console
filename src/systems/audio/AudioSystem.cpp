#include "AudioSystem.h"

#include <cstring>

#include "config/BoardConfig.h"
#include "config/Assets.h"

#include "core/SystemEventBus.h"
#include "systems/audio/AudioEvent.h"

#include "log/Logger.h"

AudioSystem::AudioSystem()
{
    driver = new MAX98357Driver(I2S_BCLK_PIN, I2S_LRCLK_PIN, I2S_DOUT_PIN);
}

AudioSystem::~AudioSystem()
{
    for (auto &entry : cache)
    {
        delete[] entry.data;
        entry.data = nullptr;
    }
    delete driver;
}

void AudioSystem::init(void *params)
{
    storage = static_cast<StorageSystem *>(params);
    if (!storage)
    {
        Log::error(Log::AUDIO_TAG, "Storage missing");
        return;
    }
    if (!driver->init(AUDIO_SAMPLE_RATE))
    {
        Log::error(Log::AUDIO_TAG, "Audio driver init failed");
        return;
    }
    mixer.init(driver);
    mixer.setMasterVolume(1.0f);
    Log::info(Log::AUDIO_TAG, "Audio system ready");
}

void AudioSystem::update()
{
    AudioEvent evt;
    while (xQueueReceive(SystemEventBus::audioQueue, &evt, 0))
    {
        switch (evt.type)
        {
        case AudioEventType::BEEP:
            beep(evt.beep.freq, evt.beep.ms);
            break;
        case AudioEventType::PLAY_WAV:
            play(evt.wav.path, false, 1);
            break;
        case AudioEventType::PLAY_LOOP_WAV:
            play(evt.wav.path, true, 1);
            break;
        case AudioEventType::STOP:
            stop();
            break;
        case AudioEventType::SET_VOLUME:
            setVolume(evt.volume.volume);
            break;
        }
    }

    //
    // The mixer owns the audio pipeline now
    //

    mixer.update();

    vTaskDelay(1);
}

void AudioSystem::play(const char *path, bool loop, uint8_t priority)
{
    if (!path)
        return;
    AudioCacheEntry *entry = findCache(path);
    if (!entry)
    {
        entry = loadCache(path);
    }
    if (!entry)
    {
        Log::error(Log::AUDIO_TAG, "Failed loading %s", path);
        return;
    }
    mixer.play(&entry->file, loop, priority, 1.0f);
}

void AudioSystem::stop()
{
    mixer.stopAll();
}

void AudioSystem::stopAll()
{
    mixer.stopAll();
}

void AudioSystem::beep(uint16_t freq, uint16_t ms)
{
    //
    // Temporary:
    // later ToneAudioSource
    //

    Log::debug(
        Log::AUDIO_TAG,
        "beep %d Hz %d ms",
        freq,
        ms);
}

void AudioSystem::setVolume(uint8_t volume)
{
    if (volume > 100)
        volume = 100;

    this->volume = volume;
    mixer.setMasterVolume(volume / 100.0f);
}

AudioCacheEntry *AudioSystem::findCache(const char *path)
{
    uint32_t now = millis();

    for (auto &entry : cache)
    {
        if (entry.data &&
            strcmp(entry.path, path) == 0)
        {
            entry.lastUsed = now;
            return &entry;
        }
    }

    return nullptr;
}

AudioCacheEntry *AudioSystem::loadCache(const char *path)
{
    IFile *file = storage->open(path);

    if (!file)
        return nullptr;

    constexpr size_t CHUNK = 1024;
    size_t capacity = CHUNK;
    size_t length = 0;
    uint8_t *buffer = new uint8_t[capacity];
    uint8_t temp[CHUNK];

    while (file->available())
    {
        int read = file->read(temp, CHUNK);

        if (read <= 0)
            break;

        if (length + read > capacity)
        {
            size_t newCapacity = capacity * 2;

            while (newCapacity < length + read)
                newCapacity *= 2;

            uint8_t *newBuffer = new uint8_t[newCapacity];
            memcpy(newBuffer, buffer, length);
            delete[] buffer;
            buffer = newBuffer;
            capacity = newCapacity;
        }

        memcpy(buffer + length, temp, read);
        length += read;
    }

    file->close();
    delete file;

    int slot = -1;
    for (int i = 0; i < AUDIO_CACHE_SIZE; i++)
    {
        if (!cache[i].data)
        {
            slot = i;
            break;
        }
    }

    // LRU eviction
    if (slot == -1)
    {
        uint32_t oldest = UINT32_MAX;

        for (int i = 0; i < AUDIO_CACHE_SIZE; i++)
        {
            if (cache[i].lastUsed < oldest)
            {
                oldest = cache[i].lastUsed;
                slot = i;
            }
        }

        delete[] cache[slot].data;
    }

    AudioCacheEntry &entry = cache[slot];
    strncpy(entry.path, path, AudioEvent::MAX_PATH_LENGTH - 1);
    entry.path[AudioEvent::MAX_PATH_LENGTH - 1] = '\0';
    entry.data = buffer;
    entry.size = length;
    entry.lastUsed = millis();
    entry.file.assign(buffer, length);

    Log::debug(Log::AUDIO_TAG, "Cached audio %s (%u bytes)", path, length);

    return &entry;
}