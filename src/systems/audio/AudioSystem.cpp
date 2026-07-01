#include "AudioSystem.h"

#include "config/BoardConfig.h"

#include "core/SystemEventBus.h"

#include "systems/audio/AudioEvent.h"

#include "log/Logger.h"

AudioSystem::AudioSystem()
{
    driver = new MAX98357Driver(
        I2S_BCLK_PIN,
        I2S_LRCLK_PIN,
        I2S_DOUT_PIN);
}

AudioSystem::~AudioSystem()
{
    stopAll();
    delete driver;
    driver = nullptr;
}

void AudioSystem::init(void *params)
{
    storage = static_cast<StorageSystem *>(params);
    if (!storage)
    {
        Log::error(Log::AUDIO_TAG, "StorageSystem missing");
        return;
    }
    if (!driver->init(AUDIO_SAMPLE_RATE))
    {
        Log::error(Log::AUDIO_TAG, "Audio driver init failed");
        return;
    }
    if (!mixer.init(driver))
    {
        Log::error(Log::AUDIO_TAG, "Mixer init failed");
        return;
    }
    if (!cache.init(storage))
    {
        Log::error(Log::AUDIO_TAG, "Audio cache init failed");
        return;
    }
    setVolume(volume);
    Log::info(Log::AUDIO_TAG, "Audio system initialized");
}

void AudioSystem::update()
{
    AudioEvent event;
    while (xQueueReceive(SystemEventBus::audioQueue, &event, 0))
    {
        switch (event.type)
        {
        case AudioEventType::PLAY_WAV:
            play(event.wav.path, false, 1);
            break;
        case AudioEventType::PLAY_LOOP_WAV:
            play(event.wav.path, true, 1);
            break;
        case AudioEventType::STOP:
            stopAll();
            break;
        case AudioEventType::SET_VOLUME:
            setVolume(event.volume.volume);
            break;
        case AudioEventType::BEEP:
            beep(event.beep.freq, event.beep.ms);
            break;
        }
    }
    mixer.update();
    vTaskDelay(1);
}

int AudioSystem::play(const char *path, bool loop, uint8_t priority)
{
    if (!path || !storage)
        return -1;
    MemoryAudioSource *source = cache.get(path);
    if (!source)
    {
        Log::error(Log::AUDIO_TAG, "Failed loading %s", path);
        return -1;
    }
    cache.retain(source);
    int voice = mixer.play(source, loop, priority, 1.0f, AudioVoice::InterruptMode::PauseResume, &cache);
    if (voice < 0)
    {
        cache.release(source);
        return -1;
    }
    Log::debug(Log::AUDIO_TAG, "Playing %s voice=%d", path, voice);
    return voice;
}

void AudioSystem::stop(int voice)
{
    mixer.stop(voice);
}

void AudioSystem::stopAll()
{
    mixer.stopAll();
}

void AudioSystem::beep(uint16_t freq, uint16_t ms)
{
    //
    // Later:
    // ToneAudioSource
    //

    Log::debug(Log::AUDIO_TAG, "Beep %uHz %ums", freq, ms);
}

void AudioSystem::setVolume(uint8_t value)
{
    if (value > 100)
        value = 100;

    volume = value;
    mixer.setMasterVolume(value / 100.0f);
    Log::debug(Log::AUDIO_TAG, "Volume %u%%", value);
}