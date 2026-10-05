#include "AudioSystem.h"

#include <cstring>

#include "config/BoardConfig.h"
#include "core/SystemEventBus.h"
#include "systems/audio/AudioEvent.h"
#include "drivers/audio/MAX98357Driver.h"
#include "log/Logger.h"
#include "drivers/storage/MemoryFile.h"
#include "drivers/bus/SPIBusLock.h"

#include "config/Assets.h"

AudioSystem::AudioSystem()
{
    driver = new MAX98357Driver(
        I2S_BCLK_PIN,  // BCLK
        I2S_LRCLK_PIN, // LRCK
        I2S_DOUT_PIN   // DOUT
    );
}

AudioSystem::~AudioSystem()
{
    for (auto &entry : cache)
    {
        delete[] entry.data;
        entry.data = nullptr;
    }
}

void AudioSystem::init(void *params)
{
    StorageSystem *storage = static_cast<StorageSystem *>(params);

    if (!storage)
    {
        Log::error(Log::AUDIO_TAG, "AudioSystem init failed: storage is null");
        return;
    }
    this->storage = storage;

    if (!driver->init())
    {
        Log::error(Log::AUDIO_TAG, "Audio driver init failed");
        return;
    }

    Log::info(Log::AUDIO_TAG, "Audio system initialized");
    driver->startWav(storage->open(AUDIO_CLICK_FILE));
}

void AudioSystem::update()
{
    AudioEvent evt;

    while (true)
    {
        while (xQueueReceive(SystemEventBus::audioQueue,
                             &evt,
                             0))
        {
            switch (evt.type)
            {
            case AudioEventType::BEEP:
                beep(evt.beep.freq, evt.beep.ms);
                break;

            case AudioEventType::PLAY_WAV:
                play(evt.wav.path);
                break;
                
            case AudioEventType::PLAY_LOOP_WAV:
                play(evt.wav.path, true);
                break;

            case AudioEventType::STOP:
                stop();
                break;

            case AudioEventType::SET_VOLUME:
                setVolume(evt.volume.volume);
                break;
            }
        }

        driver->update();

        if (looping &&
            !driver->isPlaying())
        {
            loopEntry->file.reset();
            driver->startWav(&loopEntry->file);
        }

        vTaskDelay(1);
    }
}

void AudioSystem::beep(uint16_t freq, uint16_t ms)
{
    Log::debug(Log::AUDIO_TAG, "Playing beep freq=%d ms=%d", freq, ms);
    driver->playTone(freq, ms);
}

void AudioSystem::play(const char *path, bool loop)
{
    looping = false;
    loopEntry = nullptr;

    if (!storage || !path)
        return;

    Log::info(Log::AUDIO_TAG, "Playing wav: %s", path);
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

    if(loop)
    {
        looping = true;
        loopEntry = entry;
    }

    entry->file.reset();
    driver->startWav(&entry->file);
}

void AudioSystem::stop()
{
    if (!driver)
        return;

    Log::debug(Log::AUDIO_TAG, "Stopping audio");

    looping = false;
    loopEntry = nullptr;

    driver->stop();
}

void AudioSystem::setVolume(uint8_t volume)
{
    if (!driver)
        return;

    if (volume > 100)
        volume = 100;

    this->volume = volume;

    Log::info(Log::AUDIO_TAG, "Setting volume to %d", volume);

    driver->setVolume(volume);
}

uint8_t AudioSystem::getVolume() const
{
    return volume;
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
    SPIBusLock::Guard guard;

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
