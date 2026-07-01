#pragma once

#include <cstdint>
#include <cstddef>

#include "drivers/audio/MAX98357Driver.h"
#include "systems/audio/AudioVoice.h"
#include "systems/audio/AudioSource.h"

class AudioMixer
{
public:
    static constexpr size_t MAX_VOICES = 8;
    static constexpr size_t BUFFER_SAMPLES = 256;

public:
    AudioMixer() = default;
    ~AudioMixer();

    bool init(MAX98357Driver *driver);

    /**
     * Starts playing a source.
     *
     * @return Voice index, or -1 if no voice available.
     */
    int play(
        AudioSource *source,
        bool loop,
        uint8_t priority,
        float volume,
        AudioVoice::InterruptMode mode,
        AudioCache *cache);

    void stop(int voice);

    void stopAll();

    void pause(int voice);

    void resume(int voice);

    void update();

    void setMasterVolume(float volume);
    float masterVolume() const;
    bool isPlaying(int voice) const;

private:
    int allocateVoice(uint8_t priority);
    void mixVoice(AudioVoice &voice);
    void clipOutput();
    void interruptVoices(int interrupterIndex);
    void resumeVoices(int interrupterIndex);
    bool isVoiceActive(int index) const;

private:
    MAX98357Driver *driver = nullptr;

    AudioVoice voices[MAX_VOICES];

    float masterGain = 1.0f;

    uint32_t playCounter = 0;

    int16_t voiceBuffer[BUFFER_SAMPLES];

    int16_t outputBuffer[BUFFER_SAMPLES];

    int32_t accumulator[BUFFER_SAMPLES];
};