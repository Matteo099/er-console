#pragma once
#include <cstdint>

enum class AudioEventType {
    BEEP,
    PLAY_WAV,
    STOP,
    SET_VOLUME
};

struct AudioEvent {
    AudioEventType type;

    union {
        struct {
            uint16_t freq;
            uint16_t ms;
        } beep;

        struct {
            const char* path;
        } wav;

        struct {
            uint8_t volume; // 0–100
        } volume;
    };
};
