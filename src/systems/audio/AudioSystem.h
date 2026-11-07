#pragma once

#include "core/SystemService.h"
#include "systems/storage/StorageSystem.h"
#include "drivers/audio/AudioDriver.h"
#include "AudioEvent.h"
#include "AudioMixer.h"
#include "AudioPreloads.h"
#include <atomic>
#include "core/TaskMutex.h"

constexpr size_t AUDIO_CACHE_SIZE = AUDIO_MAX_SOURCES;

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
    explicit AudioSystem(QueueHandle_t commandQueue = nullptr);
    ~AudioSystem();

    void init(void *params = nullptr) override;
    void update() override;

    AudioSourceId beep(uint16_t freq = 1000, uint16_t ms = 100, AudioChannel channel = AudioChannel::Effects, uint8_t volume = 100);

    AudioSourceId play(const char *path, bool loop = false, AudioChannel channel = AudioChannel::All, uint8_t volume = 100);
    void stop(AudioChannel channel = AudioChannel::All);
    bool stopSource(AudioSourceId id);
    bool setSourceVolume(AudioSourceId id, uint8_t volume);
    bool getSourceVolume(AudioSourceId id, uint8_t &volume) const;
    bool isPlaying(AudioSourceId id) const;
    bool preload(const char *path);
    AudioAssetId retainAsset(const char *path);
    AudioAssetState assetState(AudioAssetId id) const;
    void releaseAsset(AudioAssetId id);
    void configure(const AudioPreferences &preferences);

    void setVolume(uint8_t volume);
    uint8_t getVolume() const;

private:
    AudioCacheEntry *findCache(const char *path);
    AudioCacheEntry *loadCache(const char *path);

    void process(const AudioEvent &event);
    void playFile(const AudioEvent &event);
    void syncControls();
    bool enqueue(AudioEvent &event, uint8_t volume);
    void release(AudioSourceId id);
    mutable TaskMutex controls;
    AudioSources sources;
    mutable AudioPreloads preloads;
    AudioMixer mixer;
    AudioPreferences preferences;
    QueueHandle_t preferencesQueue = nullptr;
    QueueHandle_t commandQueue = nullptr; // Optional externally-owned queue for isolated integration tests.
    QueueHandle_t queueHandle() const { return commandQueue ? commandQueue : SystemEventBus::audioQueue; }

    StorageSystem *storage = nullptr;
    AudioDriver *driver = nullptr;

    AudioCacheEntry cache[AUDIO_CACHE_SIZE]{};
    size_t cacheBytes = 0, cacheBudget = 0;
    AudioCacheEntry *evictableCache();
    void evict(AudioCacheEntry &entry);
    bool evictable(const AudioCacheEntry &entry);

    std::atomic<uint8_t> volume{100};
    uint8_t appliedVolume = 100;
};
