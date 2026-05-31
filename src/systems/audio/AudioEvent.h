#pragma once
#include <cstdint>
#include <cstddef>

enum class AudioEventType {
    BEEP,
    PLAY_WAV,
    STOP,
    SET_VOLUME
};

struct AudioEvent {
    static constexpr std::size_t MAX_PATH_LENGTH = 128;

    AudioEventType type;

    union {
        struct {
            uint16_t freq;
            uint16_t ms;
        } beep;

        struct {
            char path[MAX_PATH_LENGTH];
        } wav;

        struct {
            uint8_t volume; // 0–100
        } volume;
    };
};
