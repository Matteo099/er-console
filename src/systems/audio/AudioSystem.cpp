#include "AudioSystem.h"

#include "config/BoardConfig.h"
#include "core/SystemEventBus.h"
#include "systems/audio/AudioEvent.h"
#include "drivers/audio/MAX98357Driver.h"
#include "log/Logger.h"

AudioSystem::AudioSystem()
{
    driver = new MAX98357Driver(
        I2S_BCLK_PIN, // BCLK
        I2S_LRCLK_PIN,  // LRCK
        I2S_DOUT_PIN  // DOUT
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
                Log::debug(Log::AUDIO_TAG, "Playing beep freq=%d ms=%d", evt.beep.freq, evt.beep.ms);
                driver->playTone(
                    evt.beep.freq,
                    evt.beep.ms);
                break;

            case AudioEventType::PLAY_WAV:
            {
                Log::info(Log::AUDIO_TAG, "Playing wav: %s", evt.wav.path);
                IFile *f = storage->open(evt.wav.path);
                driver->playWav(f);
                break;
            }

            case AudioEventType::STOP:
                Log::debug(Log::AUDIO_TAG, "Stopping audio");
                driver->stop();
                break;

            case AudioEventType::SET_VOLUME:
                Log::info(Log::AUDIO_TAG, "Setting volume to %d", evt.volume.volume);
                volume = evt.volume.volume;
                driver->setVolume(volume);
                break;
            }
        }
    }
}

void AudioSystem::beep(uint16_t freq, uint16_t ms)
{
    driver->playTone(freq, ms);
}
