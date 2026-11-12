#pragma once
#include <cstdint>

enum class AudioChannel : uint8_t { Effects, Music, All };

struct AudioChannelPreferences {
    AudioChannelPreferences(bool enabled = true, uint8_t volume = 70)
        : enabled(enabled), volume(volume) {
    }

    bool enabled;
    uint8_t volume;
};

struct AudioPreferences {
    AudioChannelPreferences music{true, 50};
    AudioChannelPreferences effects{true, 70};
};
