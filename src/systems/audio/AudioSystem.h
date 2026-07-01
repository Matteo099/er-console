#pragma once

#include "core/SystemService.h"

#include "systems/storage/StorageSystem.h"

#include "drivers/audio/MAX98357Driver.h"

#include "systems/audio/AudioMixer.h"
#include "systems/audio/AudioCache.h"

class AudioSystem : public SystemService
{
public:
    AudioSystem();
    ~AudioSystem() override;
    void init(void *params = nullptr) override;
    void update() override;
    bool play(AudioHandle &handle, const char *path, bool loop = false, uint8_t priority = 0);
    void stopAll();
    void stop(AudioHandle handle);
    void pause(AudioHandle handle);
    void resume(AudioHandle handle);
    bool isPlaying(AudioHandle handle) const;

    void beep(uint16_t freq, uint16_t ms);
    void setVolume(uint8_t volume);
    uint8_t getVolume() const { return volume; }

private:
    StorageSystem *storage = nullptr;
    MAX98357Driver *driver = nullptr;
    AudioMixer mixer;
    AudioCache cache;
    uint8_t volume = 100;
};