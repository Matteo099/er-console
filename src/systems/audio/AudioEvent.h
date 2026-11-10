#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "core/SystemEventBus.h"
#include "AudioPreferences.h"
#include "AudioSources.h"

enum class AudioEventType
{
    BEEP,
    PLAY_WAV,
    PLAY_LOOP_WAV,
    STOP,
    SET_VOLUME,
    PRELOAD
};

struct AudioEvent
{
    static constexpr std::size_t MAX_PATH_LENGTH = 128;

    AudioEventType type;
    AudioChannel channel = AudioChannel::Effects;
    AudioSourceId source = 0;
    uint32_t queuedAt = 0;

    union
    {
        struct
        {
            uint16_t freq;
            uint16_t ms;
        } beep;

        struct
        {
            char path[MAX_PATH_LENGTH];
        } wav;

        struct
        {
            uint8_t volume; // 0–100
        } volume;
    };
};

bool sendAudioEvent(const char *pathToFile, AudioEventType type = AudioEventType::PLAY_WAV, AudioChannel channel = AudioChannel::All);
AudioSourceId playAudioFile(const char *path, bool loop = false, AudioChannel channel = AudioChannel::All, uint8_t volume = 100);
bool stopAudioSource(AudioSourceId id);
bool isAudioSourcePlaying(AudioSourceId id);
