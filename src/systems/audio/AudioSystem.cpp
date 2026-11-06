#include "AudioSystem.h"
#include <algorithm>
#include <cstring>
#include <new>
#include "config/BoardConfig.h"
#include "config/Assets.h"
#include "drivers/audio/MAX98357Driver.h"
#include "drivers/bus/SPIBusLock.h"
#include "log/Logger.h"

AudioSystem::AudioSystem() : mixer(AUDIO_SAMPLE_RATE)
{
    driver = new MAX98357Driver(I2S_BCLK_PIN, I2S_LRCLK_PIN, I2S_DOUT_PIN);
    preferencesQueue = xQueueCreate(1, sizeof(AudioPreferences));
    configASSERT(preferencesQueue);
}
AudioSystem::~AudioSystem()
{
    for (auto &entry : cache) delete[] entry.data;
    delete driver;
    vQueueDelete(preferencesQueue);
}
void AudioSystem::init(void *params)
{
    storage = static_cast<StorageSystem *>(params);
    if (!storage || !driver->init()) return;
    driver->setVolume(volume.load());
    Log::info(Log::AUDIO_TAG, "Audio system initialized: independent music/effects mixer");
    sendAudioEvent(AUDIO_CLICK_FILE);
}
void AudioSystem::configure(const AudioPreferences &value) { xQueueOverwrite(preferencesQueue, &value); }
void AudioSystem::update()
{
    AudioEvent event;
    while (true)
    {
        if (xQueueReceive(preferencesQueue, &preferences, 0)) mixer.configure(preferences);
        while (xQueueReceive(SystemEventBus::audioQueue, &event, 0)) { syncStops(); process(event); }
        syncStops();
        if (!volume.load() || !mixer.available()) driver->stop();
        else
        {
            if (!driver->isPlaying()) driver->startWav(&mixer);
            driver->update();
        }
        vTaskDelay(1);
    }
}
void AudioSystem::process(const AudioEvent &event)
{
    if ((event.type == AudioEventType::PLAY_WAV || event.type == AudioEventType::PLAY_LOOP_WAV || event.type == AudioEventType::BEEP) &&
        event.epoch != audioEpoch(event.channel).load()) return;
    switch (event.type)
    {
    case AudioEventType::PLAY_WAV: playFile(event.wav.path, false, event.channel); break;
    case AudioEventType::PLAY_LOOP_WAV: playFile(event.wav.path, true, event.channel); break;
    case AudioEventType::BEEP:
        if (volume.load() && preferences.effects.enabled && preferences.effects.volume) mixer.tone(event.beep.freq, event.beep.ms);
        break;
    case AudioEventType::STOP:
        mixer.stop(event.channel);
        break;
    case AudioEventType::SET_VOLUME:
        volume.store(std::min<uint8_t>(100, event.volume.volume));
        driver->setVolume(volume.load());
        if (!volume.load()) driver->stop();
        break;
    }
}
void AudioSystem::beep(uint16_t frequency, uint16_t ms)
{
    AudioEvent event{}; event.type = AudioEventType::BEEP;
    event.epoch = audioEpoch(AudioChannel::Effects).load();
    event.beep.freq = frequency; event.beep.ms = ms;
    xQueueSend(SystemEventBus::audioQueue, &event, 0);
}
void AudioSystem::play(const char *path, bool loop, AudioChannel channel)
{
    sendAudioEvent(path, loop ? AudioEventType::PLAY_LOOP_WAV : AudioEventType::PLAY_WAV, channel);
}
void AudioSystem::stop(AudioChannel channel) { sendAudioEvent(nullptr, AudioEventType::STOP, channel); }
void AudioSystem::setVolume(uint8_t value)
{
    AudioEvent event{}; event.type = AudioEventType::SET_VOLUME; event.volume.volume = std::min<uint8_t>(100, value);
    xQueueSend(SystemEventBus::audioQueue, &event, 0);
}
uint8_t AudioSystem::getVolume() const { return volume.load(); }
void AudioSystem::syncStops()
{
    const auto musicNow = audioEpoch(AudioChannel::Music).load(), effectsNow = audioEpoch(AudioChannel::Effects).load();
    if (musicEpoch != musicNow) { musicEpoch = musicNow; mixer.stop(AudioChannel::Music); }
    if (effectsEpoch != effectsNow) { effectsEpoch = effectsNow; mixer.stop(AudioChannel::Effects); }
}
void AudioSystem::playFile(const char *path, bool loop, AudioChannel channel)
{
    const auto &settings = channel == AudioChannel::Music ? preferences.music : preferences.effects;
    if (!storage || !path || !*path) return;
    // Remember a music cue started while muted, so enabling Music can resume it.
    // Muted one-shot effects are discarded instead of playing later.
    if (channel != AudioChannel::Music && (!volume.load() || !settings.enabled || !settings.volume)) return;
    auto *entry = findCache(path);
    if (!entry) entry = loadCache(path);
    if (entry && !mixer.play(entry->data, entry->size, channel, loop))
        Log::warn(Log::AUDIO_TAG, "Unsupported WAV (16-bit PCM mono/stereo required): %s", path);
}
AudioCacheEntry *AudioSystem::findCache(const char *path)
{
    for (auto &entry : cache)
        if (entry.data && !std::strcmp(entry.path, path)) { entry.lastUsed = millis(); return &entry; }
    return nullptr;
}
AudioCacheEntry *AudioSystem::loadCache(const char *path)
{
    SPIBusLock::Guard guard;
    IFile *file = storage->open(path);
    if (!file) return nullptr;
    const size_t expected = file->length();
    auto *data = expected ? new (std::nothrow) uint8_t[expected] : nullptr;
    size_t length = 0;
    while (data && length < expected)
    {
        const int count = file->read(data + length, std::min<size_t>(1024, expected - length));
        if (count <= 0) break;
        length += count;
    }
    file->close(); delete file;
    if (!data || length != expected)
    {
        if (data) storage->noteIOFailure();
        delete[] data;
        return nullptr;
    }
    storage->noteIOSuccess();
    AudioCacheEntry *slot = nullptr;
    for (auto &entry : cache)
    {
        if (mixer.references(entry.data)) continue;
        if (!slot || !entry.data || entry.lastUsed < slot->lastUsed) slot = &entry;
        if (!entry.data) break;
    }
    if (!slot) { delete[] data; return nullptr; }
    delete[] slot->data;
    slot->data = data; slot->size = length; slot->lastUsed = millis();
    std::strncpy(slot->path, path, AudioEvent::MAX_PATH_LENGTH - 1);
    slot->path[AudioEvent::MAX_PATH_LENGTH - 1] = 0;
    return slot;
}
