#include "AudioMixer.h"

#include <cstring>
#include <algorithm>

bool AudioMixer::init(
    MAX98357Driver *drv)
{
    if (!drv)
        return false;

    driver = drv;

    for (auto &voice : voices)
    {
        voice.reset();
    }

    memset(
        accumulator,
        0,
        sizeof(accumulator));

    return true;
}

int AudioMixer::play(
    AudioSource *source,
    bool loop,
    uint8_t priority,
    float volume)
{
    if (!source)
        return -1;

    int index =
        allocateVoice(priority);

    if (index < 0)
        return -1;

    AudioVoice &voice =
        voices[index];

    voice.reset();

    voice.source = source;

    voice.loop = loop;

    voice.priority = priority;

    voice.volume = volume;

    voice.state =
        AudioVoice::State::Playing;

    source->reset();

    return index;
}

void AudioMixer::stop(
    int voice)
{
    if (voice < 0 ||
        voice >= MAX_VOICES)
        return;

    voices[voice].reset();
}

void AudioMixer::stopAll()
{
    for (auto &voice : voices)
    {
        voice.reset();
    }
}

void AudioMixer::pause(
    int voice)
{
    if (voice < 0 ||
        voice >= MAX_VOICES)
        return;

    if (voices[voice].state ==
        AudioVoice::State::Playing)
    {
        voices[voice].state =
            AudioVoice::State::Paused;
    }
}

void AudioMixer::resume(
    int voice)
{
    if (voice < 0 ||
        voice >= MAX_VOICES)
        return;

    if (voices[voice].state ==
        AudioVoice::State::Paused)
    {
        voices[voice].state =
            AudioVoice::State::Playing;
    }
}

void AudioMixer::update()
{
    if (!driver)
        return;

    memset(
        accumulator,
        0,
        sizeof(accumulator));

    for (auto &voice : voices)
    {
        if (!voice.active())
            continue;

        if (voice.state ==
            AudioVoice::State::Paused)
            continue;

        if (!voice.source)
            continue;

        memset(
            voiceBuffer,
            0,
            sizeof(voiceBuffer));

        size_t read =
            voice.source->read(
                voiceBuffer,
                BUFFER_SAMPLES);

        if (read < BUFFER_SAMPLES)
        {
            if (voice.loop)
            {
                voice.source->reset();

                read +=
                    voice.source->read(
                        voiceBuffer + read,
                        BUFFER_SAMPLES - read);
            }
            else
            {
                voice.reset();
            }
        }

        float gain =
            masterGain *
            voice.volume;

        for (size_t i = 0;
             i < BUFFER_SAMPLES;
             i++)
        {
            accumulator[i] +=
                static_cast<int32_t>(
                    voiceBuffer[i] * gain);
        }
    }

    clipOutput();

    driver->write(
        outputBuffer,
        BUFFER_SAMPLES);
}

void AudioMixer::setMasterVolume(
    float volume)
{
    if (volume < 0.0f)
        volume = 0.0f;

    if (volume > 1.0f)
        volume = 1.0f;

    masterGain = volume;
}

float AudioMixer::masterVolume() const
{
    return masterGain;
}

bool AudioMixer::isPlaying(
    int voice) const
{
    if (voice < 0 ||
        voice >= MAX_VOICES)
        return false;

    return voices[voice].state ==
           AudioVoice::State::Playing;
}

int AudioMixer::allocateVoice(
    uint8_t priority)
{
    //
    // First try free voice
    //

    for (int i = 0;
         i < MAX_VOICES;
         i++)
    {
        if (!voices[i].active())
        {
            return i;
        }
    }

    //
    // No free voice.
    // Steal lowest priority.
    //

    int candidate = -1;

    uint8_t lowest =
        255;

    for (int i = 0;
         i < MAX_VOICES;
         i++)
    {
        if (voices[i].priority < lowest)
        {
            lowest =
                voices[i].priority;

            candidate = i;
        }
    }

    if (candidate < 0)
        return -1;

    if (priority < lowest)
        return -1;

    return candidate;
}

void AudioMixer::clipOutput()
{
    for (size_t i = 0;
         i < BUFFER_SAMPLES;
         i++)
    {
        int32_t sample =
            accumulator[i];

        if (sample > 32767)
            sample = 32767;

        else if (sample < -32768)
            sample = -32768;

        outputBuffer[i] =
            static_cast<int16_t>(
                sample);
    }
}