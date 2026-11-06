#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include "core/SystemEventBus.h"
#include "AudioPreferences.h"
#include <atomic>

inline std::atomic<uint32_t> &audioEpoch(AudioChannel channel)
{
    static std::atomic<uint32_t> music{0}, effects{0};
    return channel == AudioChannel::Music ? music : effects;
}

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
    AudioChannel channel = AudioChannel::Effects;
    uint32_t epoch = 0;

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

inline bool sendAudioEvent(const char *pathToFile, AudioEventType type = AudioEventType::PLAY_WAV, AudioChannel channel = AudioChannel::All)
{
    if (type == AudioEventType::STOP)
    {
        if (channel != AudioChannel::Effects) audioEpoch(AudioChannel::Music).fetch_add(1);
        if (channel != AudioChannel::Music) audioEpoch(AudioChannel::Effects).fetch_add(1);
        return true; // Stop cannot be lost behind a full sound-effect queue.
    }
    AudioEvent evt{};
    evt.type = type;
    evt.channel = channel == AudioChannel::All && type != AudioEventType::STOP ?
                  (type == AudioEventType::PLAY_LOOP_WAV ? AudioChannel::Music : AudioChannel::Effects) : channel;
    evt.epoch = audioEpoch(evt.channel).load();

    std::strncpy(
        evt.wav.path,
        pathToFile ? pathToFile : "",
        AudioEvent::MAX_PATH_LENGTH - 1);

    evt.wav.path[AudioEvent::MAX_PATH_LENGTH - 1] = '\0';

    if (!SystemEventBus::audioQueue) return false;
    return xQueueSend(SystemEventBus::audioQueue, &evt, 0) == pdPASS;
}
