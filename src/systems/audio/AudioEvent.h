#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "core/SystemEventBus.h"
#include "AudioHandle.h"

enum class AudioEventType
{
    BEEP,
    PLAY,
    STOP,
    STOP_ALL,
    PAUSE,
    RESUME,
    SET_VOLUME
};

struct AudioEvent
{
    static constexpr std::size_t MAX_PATH_LENGTH = 128;

    AudioEventType type;
    AudioHandle handle;

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
            bool loop;
            uint8_t priority;
        } wav;

        struct
        {
            uint8_t volume; // 0–100
        } volume;
    };
};

inline void createAudioHandle(AudioHandle &handle)
{
    static uint16_t nextId = 1;
    handle.id = nextId++;
}

inline AudioHandle createAudioHandle()
{
    AudioHandle handle;
    createAudioHandle(handle);
    return handle;
}

inline AudioHandle playAudioEvent(const char *pathToFile, AudioEventType type = AudioEventType::PLAY, bool loop = false, uint8_t priority = 1)
{
    AudioHandle handle = createAudioHandle();
    AudioEvent evt{};
    evt.type = type;
    evt.handle = handle;

    std::strncpy(
        evt.wav.path,
        pathToFile,
        AudioEvent::MAX_PATH_LENGTH - 1);

    evt.wav.path[AudioEvent::MAX_PATH_LENGTH - 1] = '\0';
    evt.wav.priority = priority;
    evt.wav.loop = loop;

    xQueueSend(
        SystemEventBus::audioQueue,
        &evt,
        portMAX_DELAY) == pdPASS;

    return handle;
}

inline bool pauseAudioEvent(AudioHandle handle)
{
    AudioEvent evt{};
    evt.type = AudioEventType::PAUSE;
    evt.handle = handle;

    return xQueueSend(
               SystemEventBus::audioQueue,
               &evt,
               portMAX_DELAY) == pdPASS;
}

inline bool resumeAudioEvent(AudioHandle handle)
{
    AudioEvent evt{};
    evt.type = AudioEventType::RESUME;
    evt.handle = handle;

    return xQueueSend(
               SystemEventBus::audioQueue,
               &evt,
               portMAX_DELAY) == pdPASS;
}