#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "core/SystemEventBus.h"

enum class AudioEventType
{
    BEEP,
    PLAY_WAV,
    PLAY_LOOP_WAV,
    STOP,
    SET_VOLUME
};

struct AudioEvent
{
    static constexpr std::size_t MAX_PATH_LENGTH = 128;

    AudioEventType type;

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

inline bool sendAudioEvent(const char *pathToFile, AudioEventType type = AudioEventType::PLAY_WAV)
{
    AudioEvent evt{};
    evt.type = type;

    std::strncpy(
        evt.wav.path,
        pathToFile,
        AudioEvent::MAX_PATH_LENGTH - 1);

    evt.wav.path[AudioEvent::MAX_PATH_LENGTH - 1] = '\0';

    return xQueueSend(
               SystemEventBus::audioQueue,
               &evt,
               portMAX_DELAY) == pdPASS;
}
