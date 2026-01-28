#include "AudioSystem.h"

#include "core/SystemEventBus.h"
#include "systems/audio/AudioEvent.h"
#include "drivers/audio/MAX98357Driver.h"
#include "log/Logger.h"

AudioSystem::AudioSystem()
{
    driver = new MAX98357Driver(
        GPIO_NUM_4, // BCLK
        GPIO_NUM_5, // LRCK
        GPIO_NUM_6  // DOUT
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
                driver->playTone(
                    evt.beep.freq,
                    evt.beep.ms);
                break;

            case AudioEventType::PLAY_WAV:
            {
                IFile *f = storage->open(evt.wav.path);
                driver->playWav(f);
                break;
            }

            case AudioEventType::STOP:
                driver->stop();
                break;

            case AudioEventType::SET_VOLUME:
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
