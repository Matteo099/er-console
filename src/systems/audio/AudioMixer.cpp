#include "AudioMixer.h"

#include <cstring>
#include <algorithm>
#include "AudioCache.h"

AudioMixer::~AudioMixer()
{
    stopAll();
}

bool AudioMixer::init(MAX98357Driver *drv)
{
    if (!drv)
        return false;

    driver = drv;
    playCounter = 0;

    for (auto &voice : voices)
    {
        voice.reset();
    }

    memset(accumulator, 0, sizeof(accumulator));
    memset(outputBuffer, 0, sizeof(outputBuffer));
    return true;
}

int AudioMixer::play(
    AudioSource *source,
    bool loop,
    uint8_t priority,
    float volume,
    AudioVoice::InterruptMode mode,
    AudioCache *cache)
{
    if (!driver || !source)
        return -1;

    int index = allocateVoice(priority);

    if (index < 0)
        return -1;

    AudioVoice &voice = voices[index];

    //
    // If replacing an existing voice,
    // release its resources first.
    //
    if (voice.active())
    {
        voice.reset();
    }

    if (volume < 0.0f)
        volume = 0.0f;

    else if (volume > 1.0f)
        volume = 1.0f;

    voice.source = source;
    voice.cache = cache;
    voice.loop = loop;
    voice.priority = priority;
    voice.volume = volume;
    voice.interruptMode = mode;
    voice.state = AudioVoice::State::Playing;
    voice.interruptedBy = 255;

    //
    // Start from beginning
    //
    source->reset();

    //
    // Register cache usage
    //
    if (cache)
    {
        cache->retain(
            reinterpret_cast<MemoryAudioSource *>(source));
    }

    //
    // Pause/stop lower priority voices
    //
    interruptVoices(index);
    voice.playId = ++playCounter;
    return index;
}

void AudioMixer::stop(int index)
{
    if (index < 0 || index >= MAX_VOICES)
    {
        return;
    }

    AudioVoice &voice = voices[index];
    if (!voice.active())
        return;

    resumeVoices(index);
    voice.reset();
}

void AudioMixer::stopAll()
{
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        stop(i);
    }
}

void AudioMixer::pause(int index)
{
    if (index < 0 || index >= MAX_VOICES)
    {
        return;
    }

    AudioVoice &voice = voices[index];
    if (voice.state == AudioVoice::State::Playing)
    {
        voice.state = AudioVoice::State::Paused;
    }
}

void AudioMixer::resume(int index)
{
    if (index < 0 || index >= MAX_VOICES)
    {
        return;
    }

    AudioVoice &voice = voices[index];
    if (voice.state == AudioVoice::State::Paused)
    {
        voice.state = AudioVoice::State::Playing;
        voice.interruptedBy = -1;
    }
}

void AudioMixer::setMasterVolume(float volume)
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

bool AudioMixer::isPlaying(int index) const
{
    if (index < 0 || index >= MAX_VOICES)
    {
        return false;
    }

    return voices[index].state == AudioVoice::State::Playing;
}

int AudioMixer::allocateVoice(uint8_t priority)
{
    //
    // First look for a free voice.
    //
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (!voices[i].active())
        {
            return i;
        }
    }

    //
    // Steal the oldest voice
    // having the lowest priority.
    //
    int candidate = -1;
    uint8_t lowestPriority = UINT8_MAX;
    uint32_t oldestPlayId = UINT32_MAX;
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        const AudioVoice &voice = voices[i];

        if (voice.priority > priority)
            continue;

        if (voice.priority < lowestPriority)
        {
            lowestPriority = voice.priority;
            oldestPlayId = voice.playId;
            candidate = i;
            continue;
        }

        if (voice.priority == lowestPriority)
        {
            if (voice.playId < oldestPlayId)
            {
                oldestPlayId = voice.playId;
                candidate = i;
            }
        }
    }

    return candidate;
}

void AudioMixer::update()
{
    if (!driver)
        return;

    memset(accumulator, 0, sizeof(accumulator));
    bool hasAudio = false;
    for (auto &voice : voices)
    {
        if (!voice.active())
            continue;

        if (voice.state != AudioVoice::State::Playing)
            continue;

        if (!voice.source)
            continue;

        mixVoice(voice);
        hasAudio = true;
    }

    clipOutput();
    driver->write(outputBuffer, BUFFER_SAMPLES);
}

void AudioMixer::mixVoice(AudioVoice &voice)
{
    int16_t buffer[BUFFER_SAMPLES];
    size_t read = voice.source->read(buffer, BUFFER_SAMPLES);

    //
    // End of stream
    //
    if (read < BUFFER_SAMPLES)
    {
        if (voice.loop)
        {
            voice.source->reset();
            read += voice.source->read(buffer + read, BUFFER_SAMPLES - read);
        }
        else
        {
            const int index = static_cast<int>(&voice - voices);
            resumeVoices(index);
            voice.reset();
            return;
        }
    }

    //
    // Fade update
    //
    if (voice.gain != voice.targetGain)
    {
        if (voice.gain < voice.targetGain)
        {
            voice.gain += voice.gainStep;

            if (voice.gain > voice.targetGain)
                voice.gain = voice.targetGain;
        }
        else
        {
            voice.gain -= voice.gainStep;

            if (voice.gain < voice.targetGain)
                voice.gain = voice.targetGain;
        }
    }

    const float gain = masterGain * voice.volume * voice.gain;
    for (size_t i = 0; i < read; ++i)
    {
        accumulator[i] += static_cast<int32_t>(buffer[i] * gain);
    }
}

void AudioMixer::clipOutput()
{
    for (size_t i = 0; i < BUFFER_SAMPLES; ++i)
    {
        int32_t sample = accumulator[i];

        if (sample > 32767)
            sample = 32767;

        else if (sample < -32768)
            sample = -32768;

        outputBuffer[i] = static_cast<int16_t>(sample);
    }
}

bool AudioMixer::isVoiceActive(int index) const
{
    if (index < 0 || index >= MAX_VOICES)
    {
        return false;
    }

    return voices[index].state == AudioVoice::State::Playing;
}

void AudioMixer::interruptVoices(int interrupterIndex)
{
    AudioVoice &interrupter = voices[interrupterIndex];
    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (i == interrupterIndex)
            continue;

        AudioVoice &voice = voices[i];
        if (!voice.active())
            continue;

        if (voice.state != AudioVoice::State::Playing)
            continue;

        //
        // Only interrupt lower priorities.
        //
        if (voice.priority >=
            interrupter.priority)
        {
            continue;
        }

        switch (voice.interruptMode)
        {
        case AudioVoice::InterruptMode::Stop:
            voice.reset();
            break;

        case AudioVoice::InterruptMode::PauseResume:
            voice.state = AudioVoice::State::Paused;
            voice.interruptedBy = interrupterIndex;
            break;
        }
    }
}

void AudioMixer::resumeVoices(int interrupterIndex)
{
    for (auto &voice : voices)
    {
        if (voice.state != AudioVoice::State::Paused)
        {
            continue;
        }

        if (voice.interruptedBy != interrupterIndex)
        {
            continue;
        }

        //
        // Is there still any playing voice
        // having a higher priority?
        //
        bool blocked = false;
        for (const auto &other : voices)
        {
            if (!other.active())
                continue;

            if (other.state != AudioVoice::State::Playing)
                continue;

            if (other.priority > voice.priority)
            {
                blocked = true;
                break;
            }
        }

        if (blocked)
            continue;

        voice.state = AudioVoice::State::Playing;
        voice.interruptedBy = -1;
    }
}