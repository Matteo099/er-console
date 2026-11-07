#include "AudioSystem.h"
#include <algorithm>
#include <cstring>
#include <esp_heap_caps.h>
#include "config/BoardConfig.h"
#include "config/Assets.h"
#include "drivers/audio/MAX98357Driver.h"
#include "drivers/bus/SPIBusLock.h"
#include "log/Logger.h"

AudioSystem::AudioSystem(QueueHandle_t queue) : mixer(AUDIO_SAMPLE_RATE), commandQueue(queue)
{
    driver = new MAX98357Driver(I2S_BCLK_PIN,I2S_LRCLK_PIN,I2S_DOUT_PIN);
    preferencesQueue = xQueueCreate(1,sizeof(AudioPreferences));
    configASSERT(preferencesQueue);
}
AudioSystem::~AudioSystem()
{
    for (auto &entry : cache) heap_caps_free(entry.data);
    delete driver;
    vQueueDelete(preferencesQueue);
}
void AudioSystem::init(void *params)
{
    storage = static_cast<StorageSystem *>(params);
    if (!storage || !driver->init()) return;
    cacheBudget = psramFound() ? 4*1024*1024 : std::min<size_t>(128*1024,ESP.getFreeHeap()/3);
    driver->setVolume(volume.load());
    appliedVolume = volume.load();
    Log::info(Log::AUDIO_TAG,"Audio ready: %u sources, %u-byte PCM cache, %u Hz mono",unsigned(AUDIO_MAX_SOURCES),unsigned(cacheBudget),unsigned(AUDIO_SAMPLE_RATE));
    play(AUDIO_CLICK_FILE);
}
void AudioSystem::configure(const AudioPreferences &value) { xQueueOverwrite(preferencesQueue,&value); }
void AudioSystem::update()
{
    AudioEvent event;
    while (true)
    {
        if (xQueueReceive(preferencesQueue,&preferences,0)) mixer.configure(preferences);
        // Bound queue work per iteration so effects cannot starve I2S.
        for (unsigned i=0;i<16 && xQueueReceive(queueHandle(),&event,0);++i)
        {
            syncControls(); process(event);
        }
        syncControls();
        const auto master = volume.load();
        if (master != appliedVolume) { appliedVolume = master; driver->setVolume(master); }
        if (!master || !mixer.available()) driver->stop();
        else { if (!driver->isPlaying()) driver->startWav(&mixer); driver->update(); }
        vTaskDelay(1);
    }
}
bool AudioSystem::enqueue(AudioEvent &event, uint8_t level)
{
    event.queuedAt = millis();
    {
        TaskMutex::Guard lock(controls);
        event.source = sources.reserve(event.channel,std::min<uint8_t>(100,level));
    }
    if (!event.source) return false;
    if (queueHandle() && xQueueSend(queueHandle(),&event,0) == pdPASS) return true;
    release(event.source); return false;
}
AudioSourceId AudioSystem::play(const char *path, bool loop, AudioChannel channel, uint8_t level)
{
    if (!path || !*path || std::strlen(path) >= AudioEvent::MAX_PATH_LENGTH) return 0;
    AudioEvent event{};
    event.type = loop ? AudioEventType::PLAY_LOOP_WAV : AudioEventType::PLAY_WAV;
    event.channel = channel == AudioChannel::All ? (loop ? AudioChannel::Music : AudioChannel::Effects) : channel;
    std::strcpy(event.wav.path,path);
    return enqueue(event,level) ? event.source : 0;
}
AudioSourceId AudioSystem::beep(uint16_t frequency, uint16_t ms, AudioChannel channel, uint8_t level)
{
    AudioEvent event{}; event.type = AudioEventType::BEEP;
    event.channel = channel == AudioChannel::All ? AudioChannel::Effects : channel;
    event.beep.freq = frequency; event.beep.ms = ms;
    return enqueue(event,level) ? event.source : 0;
}
bool AudioSystem::preload(const char *path)
{
    if (!path || !*path || std::strlen(path) >= AudioEvent::MAX_PATH_LENGTH || !queueHandle()) return false;
    AudioEvent event{}; event.type = AudioEventType::PRELOAD; std::strcpy(event.wav.path,path);
    return xQueueSend(queueHandle(),&event,0) == pdPASS;
}
AudioAssetId AudioSystem::retainAsset(const char *path)
{
    TaskMutex::Guard lock(controls);
    bool created = false;
    const auto id = preloads.retain(path,created);
    if (!id || !created) return id;
    AudioEvent event{}; event.type = AudioEventType::PRELOAD; event.source = id;
    std::strcpy(event.wav.path,path);
    if (queueHandle() && xQueueSend(queueHandle(),&event,0) == pdPASS) return id;
    preloads.release(id); return 0;
}
AudioAssetState AudioSystem::assetState(AudioAssetId id) const
{
    TaskMutex::Guard lock(controls); return preloads.state(id);
}
void AudioSystem::releaseAsset(AudioAssetId id) { TaskMutex::Guard lock(controls); preloads.release(id); }
void AudioSystem::release(AudioSourceId id) { TaskMutex::Guard lock(controls); sources.release(id); }
void AudioSystem::stop(AudioChannel channel) { TaskMutex::Guard lock(controls); sources.stop(channel); }
bool AudioSystem::stopSource(AudioSourceId id) { TaskMutex::Guard lock(controls); return sources.release(id); }
bool AudioSystem::setSourceVolume(AudioSourceId id, uint8_t level)
{
    TaskMutex::Guard lock(controls);
    if (auto *source = sources.find(id)) { source->volume = std::min<uint8_t>(100,level); return true; }
    return false;
}
bool AudioSystem::getSourceVolume(AudioSourceId id, uint8_t &level) const
{
    TaskMutex::Guard lock(controls);
    if (auto *source = sources.find(id)) { level = source->volume; return true; }
    return false;
}
bool AudioSystem::isPlaying(AudioSourceId id) const
{
    TaskMutex::Guard lock(controls);
    return sources.find(id) != nullptr;
}
void AudioSystem::setVolume(uint8_t level) { volume.store(std::min<uint8_t>(100,level)); }
uint8_t AudioSystem::getVolume() const { return volume.load(); }
void AudioSystem::syncControls()
{
    TaskMutex::Guard lock(controls);
    // Stop is a mailbox operation, not a queue item. A reused slot gets a new
    // generation, so old queued commands cannot act on its replacement.
    for (const auto id : mixer.sourceIds()) if (id && !sources.find(id)) mixer.stopSource(id);
    for (const auto &source : sources.slots())
    {
        if (!source.id || !source.started) continue;
        if (!mixer.isPlaying(source.id)) sources.release(source.id);
        else mixer.setSourceVolume(source.id,source.volume);
    }
}
void AudioSystem::process(const AudioEvent &event)
{
    if (event.type == AudioEventType::SET_VOLUME) { setVolume(event.volume.volume); return; }
    if (event.type == AudioEventType::STOP) { stop(event.channel); return; }
    if (event.type == AudioEventType::PRELOAD)
    {
        if (event.source)
        {
            TaskMutex::Guard lock(controls);
            if (!preloads.find(event.source)) return; // Scene left before this queued load.
        }
        auto *entry = findCache(event.wav.path);
        if (!entry) entry = loadCache(event.wav.path);
        if (event.source)
        {
            TaskMutex::Guard lock(controls); preloads.finish(event.source,entry != nullptr);
        }
        Log::debug(Log::AUDIO_TAG,"Preload %s: %s",event.wav.path,entry ? "ready" : "failed");
        return;
    }
    {
        TaskMutex::Guard lock(controls);
        if (!sources.find(event.source)) return;
    }
    if (event.type == AudioEventType::BEEP)
    {
        // A slider may configure the bus after this iteration's first poll
        // but before queuing its preview. Use that new gain for the first PCM.
        if (xQueueReceive(preferencesQueue,&preferences,0)) mixer.configure(preferences);
        TaskMutex::Guard lock(controls);
        auto *source = sources.find(event.source);
        if (!source) return;
        const auto &settings = event.channel == AudioChannel::Music ? preferences.music : preferences.effects;
        if (volume.load() && settings.enabled && settings.volume &&
            mixer.tone(event.beep.freq,event.beep.ms,event.source,source->volume,event.channel)) source->started = true;
        else sources.release(event.source);
    }
    else playFile(event);
}
void AudioSystem::playFile(const AudioEvent &event)
{
    const bool loop = event.type == AudioEventType::PLAY_LOOP_WAV;
    const auto &settings = event.channel == AudioChannel::Music ? preferences.music : preferences.effects;
    if (!storage || (!loop && event.channel == AudioChannel::Effects && (!volume.load() || !settings.enabled || !settings.volume)))
    {
        release(event.source); return;
    }
    auto *entry = findCache(event.wav.path);
    const bool cacheHit = entry != nullptr;
    if (!entry) entry = loadCache(event.wav.path);
    syncControls(); // A stop may have arrived while SD was being read.
    {
        TaskMutex::Guard lock(controls);
        auto *source = sources.find(event.source);
        if (!source) return;
        if (entry && mixer.play(entry->data,entry->size,event.channel,loop,event.source,source->volume)) source->started = true;
        else
        {
            Log::warn(Log::AUDIO_TAG,"Cannot play WAV source %lu: %s (file I/O, PCM format or source limit)",static_cast<unsigned long>(event.source),event.wav.path);
            sources.release(event.source); return;
        }
    }
    Log::debug(Log::AUDIO_TAG,"WAV started: %s cache=%s delay=%lu ms",event.wav.path,cacheHit ? "hit" : "miss",static_cast<unsigned long>(millis()-event.queuedAt));
}
AudioCacheEntry *AudioSystem::findCache(const char *path)
{
    for (auto &entry : cache)
        if (entry.data && !std::strcmp(entry.path,path)) { entry.lastUsed = millis(); return &entry; }
    return nullptr;
}
AudioCacheEntry *AudioSystem::evictableCache()
{
    AudioCacheEntry *slot = nullptr;
    for (auto &entry : cache)
    {
        if (!entry.data) return &entry;
        if (evictable(entry) && (!slot || entry.lastUsed < slot->lastUsed)) slot = &entry;
    }
    return slot;
}
bool AudioSystem::evictable(const AudioCacheEntry &entry)
{
    if (mixer.references(entry.data)) return false;
    TaskMutex::Guard lock(controls); return !preloads.retained(entry.path);
}
void AudioSystem::evict(AudioCacheEntry &entry)
{
    cacheBytes -= entry.size; heap_caps_free(entry.data); entry = AudioCacheEntry{};
}
AudioCacheEntry *AudioSystem::loadCache(const char *path)
{
    if (!storage || !cacheBudget) return nullptr;
    IFile *file = nullptr;
    size_t expected = 0;
    {
        SPIBusLock::Guard guard;
        file = storage->open(path);
        if (!file) return nullptr;
        expected = file->length();
    }
    const auto closeFile = [&]() { SPIBusLock::Guard guard; file->close(); delete file; };
    auto *slot = evictableCache();
    if (!expected || expected > cacheBudget || !slot)
    {
        Log::warn(Log::AUDIO_TAG,"WAV exceeds available cache: %s (%u bytes; budget %u)",path,unsigned(expected),unsigned(cacheBudget));
        closeFile(); return nullptr;
    }
    evict(*slot);
    while (cacheBytes + expected > cacheBudget)
    {
        AudioCacheEntry *oldest = nullptr;
        for (auto &entry : cache)
            if (entry.data && evictable(entry) && (!oldest || entry.lastUsed < oldest->lastUsed)) oldest = &entry;
        if (!oldest) { closeFile(); return nullptr; }
        evict(*oldest);
    }
    const uint32_t caps = MALLOC_CAP_8BIT | (psramFound() ? MALLOC_CAP_SPIRAM : MALLOC_CAP_INTERNAL);
    auto *data = static_cast<uint8_t *>(heap_caps_malloc(expected,caps));
    size_t length = 0, yieldedAt = 0;
    while (data && length < expected)
    {
        const size_t requested = std::min<size_t>(1024,expected-length);
        int count;
        // Let display/touch/other storage clients run between SD blocks.
        { SPIBusLock::Guard guard; count = file->read(data+length,requested); }
        if (count <= 0 || size_t(count) > requested) break;
        length += count;
        // SD SPI transfers can busy-wait. A batch of several large WAVs must
        // periodically let this core's idle task feed the task watchdog.
        if (length-yieldedAt >= 32*1024) { vTaskDelay(1); yieldedAt = length; }
    }
    closeFile();
    if (!data || length != expected)
    {
        if (data) storage->noteIOFailure();
        Log::warn(Log::AUDIO_TAG,"Incomplete WAV load: %s (%u/%u bytes)",path,unsigned(length),unsigned(expected));
        heap_caps_free(data); return nullptr;
    }
    storage->noteIOSuccess();
    AudioMixer validator(AUDIO_SAMPLE_RATE);
    if (!validator.play(data,length,AudioChannel::Effects,false))
    {
        Log::warn(Log::AUDIO_TAG,"Invalid WAV preload: %s",path);
        heap_caps_free(data); return nullptr;
    }
    slot->data = data; slot->size = length; slot->lastUsed = millis();
    std::strcpy(slot->path,path); cacheBytes += length;
    return slot;
}
