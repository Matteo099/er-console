#pragma once

#include "core/SystemService.h"
#include "systems/storage/StorageSystem.h"
#include "drivers/audio/AudioDriver.h"
#include "AudioEvent.h"
#include "AudioMixer.h"
#include <atomic>

#define AUDIO_CACHE_SIZE 4

struct AudioCacheEntry
{
    char path[AudioEvent::MAX_PATH_LENGTH];
    uint8_t *data = nullptr;
    size_t size = 0;
    uint32_t lastUsed = 0;
};

class AudioSystem : public SystemService
{
public:
    AudioSystem();
    ~AudioSystem();

    void init(void *params = nullptr) override;
    void update() override;

    void beep(uint16_t freq = 1000, uint16_t ms = 100);

    void play(const char *path, bool loop = false, AudioChannel channel = AudioChannel::All);
    void stop(AudioChannel channel = AudioChannel::All);
    void configure(const AudioPreferences &preferences);

    void setVolume(uint8_t volume);
    uint8_t getVolume() const;

private:
    AudioCacheEntry *findCache(const char *path);
    AudioCacheEntry *loadCache(const char *path);

    void process(const AudioEvent &event);
    void playFile(const char *path, bool loop, AudioChannel channel);
    void syncStops();
    uint32_t musicEpoch = 0, effectsEpoch = 0;
    AudioMixer mixer;
    AudioPreferences preferences;
    QueueHandle_t preferencesQueue = nullptr;

    StorageSystem *storage = nullptr;
    AudioDriver *driver = nullptr;

    AudioCacheEntry cache[AUDIO_CACHE_SIZE]{};

    std::atomic<uint8_t> volume{100};
};
