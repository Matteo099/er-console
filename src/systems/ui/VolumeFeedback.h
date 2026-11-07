#pragma once
#include "systems/audio/AudioSources.h"

class AudioSystem;

// Scene-owned preview: one short source, with a shared rate limit for both
// volume sliders. Views publish intent and never access the audio driver.
class VolumeFeedback
{
public:
    static constexpr uint32_t INTERVAL_MS = 250;
    static constexpr uint16_t FREQUENCY_HZ = 750, DURATION_MS = 40;
    static constexpr uint8_t SOURCE_VOLUME = 50;

    explicit VolumeFeedback(AudioSystem *system = nullptr) : audio(system) {}
    ~VolumeFeedback() { stop(); }
    VolumeFeedback(const VolumeFeedback &) = delete;
    VolumeFeedback &operator=(const VolumeFeedback &) = delete;
    void bind(AudioSystem *system);
    void request(AudioChannel channel, AudioChannelPreferences value, uint32_t now);
    void update(uint32_t now);
    void stop();

private:
    AudioSystem *audio = nullptr;
    AudioSourceId source = 0;
    AudioChannel channel = AudioChannel::Effects, lastChannel = AudioChannel::Effects;
    uint8_t volume = 0, lastVolume = 0;
    uint32_t attemptedAt = 0;
    bool attempted = false, pending = false, heard = false;
};
