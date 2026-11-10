#pragma once
#include "AudioPreferences.h"
#include <array>
#include <cstddef>
#include <cstdint>

using AudioSourceId = uint32_t;
constexpr size_t AUDIO_MAX_SOURCES = 8;

// Task-only control mailbox. AudioSystem protects it with its short control
// mutex; rendering, storage access and I2S never run on a caller's task.
class AudioSources
{
public:
    struct Source
    {
        AudioSourceId id = 0;
        AudioChannel channel = AudioChannel::Effects;
        uint8_t volume = 100;
        bool started = false;
    };
    AudioSourceId reserve(AudioChannel channel, uint8_t volume)
    {
        for (size_t i=0;i<sources.size();++i)
            if (!sources[i].id)
            {
                generation = (generation + 1) & 0x07ffffff; // Positive handles also fit 32-bit Lua integers.
                if (!generation) ++generation;
                auto &source = sources[i];
                source.id = (generation << 4) | uint32_t(i+1);
                source.channel = channel; source.volume = volume; source.started = false;
                return source.id;
            }
        return 0;
    }
    Source *find(AudioSourceId id)
    {
        const auto slot = id & 15;
        if (!slot || slot > sources.size()) return nullptr;
        auto &source = sources[slot-1];
        return source.id == id ? &source : nullptr;
    }
    bool release(AudioSourceId id)
    {
        if (auto *source = find(id)) { *source = Source{}; return true; }
        return false;
    }
    const Source *find(AudioSourceId id) const { return const_cast<AudioSources *>(this)->find(id); }
    void stop(AudioChannel channel)
    {
        for (auto &source : sources)
            if (channel == AudioChannel::All || source.channel == channel) source = Source{};
    }
    const std::array<Source,AUDIO_MAX_SOURCES> &slots() const { return sources; }
private:
    std::array<Source,AUDIO_MAX_SOURCES> sources{};
    uint32_t generation = 0;
};
