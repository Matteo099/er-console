#include "AudioSystem.h"

#include "config/BoardConfig.h"
#include "core/SystemEventBus.h"
#include "systems/audio/AudioEvent.h"
#include "drivers/audio/MAX98357Driver.h"
#include "log/Logger.h"

#include "config/Assets.h"

AudioSystem::AudioSystem()
{
    driver = new MAX98357Driver(
        I2S_BCLK_PIN,  // BCLK
        I2S_LRCLK_PIN, // LRCK
        I2S_DOUT_PIN   // DOUT
    );
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
    driver->playWav(storage->open(AUDIO_CLICK_FILE));
}

void AudioSystem::update()
{
    AudioEvent evt;

    while (true)
    {
        if (xQueueReceive(SystemEventBus::audioQueue, &evt, portMAX_DELAY))
        {
            switch (evt.type)
            {
            case AudioEventType::BEEP:
                beep(evt.beep.freq, evt.beep.ms);
                break;

            case AudioEventType::PLAY_WAV:
                play(evt.wav.path);
                break;

            case AudioEventType::STOP:
                stop();
                break;

            case AudioEventType::SET_VOLUME:
                setVolume(evt.volume.volume);
                break;
            }
        }
    }
}

void AudioSystem::beep(uint16_t freq, uint16_t ms)
{
    Log::debug(Log::AUDIO_TAG, "Playing beep freq=%d ms=%d", freq, ms);
    driver->playTone(freq, ms);
}

void AudioSystem::play(const char *path)
{
    if (!storage || !path)
        return;

    Log::info(Log::AUDIO_TAG, "Playing wav: %s", path);

    IFile *file = storage->open(path);

    if (!file)
    {
        Log::error(Log::AUDIO_TAG, "Unable to open audio file: %s", path);
        return;
    }

    driver->playWav(file);
}

void AudioSystem::stop()
{
    if (!driver)
        return;

    Log::debug(Log::AUDIO_TAG, "Stopping audio");

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
