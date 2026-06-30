#pragma once

#include <vector>

#include "core/SystemService.h"
#include "systems/storage/StorageSystem.h"
#include "drivers/audio/MAX98357Driver.h"

#include "AudioMixer.h"
#include "AudioCacheEntry.h"

class AudioSystem : public SystemService
{
public:
    AudioSystem();
    ~AudioSystem();
    void init(void *params = nullptr) override;
    void update() override;

private:
    void play(const char *path, bool loop = false, uint8_t priority = 0);
    void stop();
    void stopAll();
    void beep(uint16_t freq, uint16_t ms);
    void setVolume(uint8_t volume);
    AudioCacheEntry *findCache(const char *path);
    AudioCacheEntry *loadCache(const char *path);

private:
    StorageSystem *storage = nullptr;
    MAX98357Driver *driver = nullptr;
    AudioMixer mixer;
    std::vector<AudioCacheEntry> cache;
    uint8_t volume = 100;
};