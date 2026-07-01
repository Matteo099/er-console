#pragma once

#include <cstdint>
#include "systems/audio/AudioHandle.h"

class AudioSource;
class AudioCache;

class AudioVoice
{
public:
    enum class State
    {
        Stopped,
        Playing,
        Paused
    };

    enum class InterruptMode
    {
        Stop,
        PauseResume
    };

public:
    void reset();

    bool active() const
    {
        return state != State::Stopped;
    }

public:
    AudioHandle handle;
    AudioSource *source = nullptr;
    AudioCache *cache = nullptr;
    State state = State::Stopped;
    bool loop = false;
    uint8_t priority = 0;
    float volume = 1.0f;
    uint32_t playId = 0;

    //
    // fade support
    //
    float gain = 1.0f;
    float targetGain = 1.0f;
    float gainStep = 0.0f;
    InterruptMode interruptMode = InterruptMode::Stop;
    uint8_t interruptedBy = 255;
};