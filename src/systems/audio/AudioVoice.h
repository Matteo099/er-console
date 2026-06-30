#pragma once

#include <cstdint>

class AudioSource;

class AudioVoice
{
public:

    enum class State
    {
        Stopped,
        Playing,
        Paused
    };

public:

    void reset();

    bool active() const
    {
        return state != State::Stopped;
    }

public:

    AudioSource* source = nullptr;

    State state = State::Stopped;

    bool loop = false;

    uint8_t priority = 0;

    float volume = 1.0f;

    // fade

    float gain = 1.0f;

    float targetGain = 1.0f;

    float gainStep = 0.0f;
};