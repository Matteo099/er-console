#include "AudioMixer.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace
{
    uint16_t u16(const uint8_t *p) { return uint16_t(p[0]) | uint16_t(p[1]) << 8; }
    uint32_t u32(const uint8_t *p) { return uint32_t(u16(p)) | uint32_t(u16(p + 2)) << 16; }
}

AudioSourceId AudioMixer::play(const uint8_t *wav, size_t length, AudioChannel channel, bool loop, AudioSourceId id, uint8_t volume)
{
    if (!outputRate || !wav || length < 12 || std::memcmp(wav, "RIFF", 4) || std::memcmp(wav + 8, "WAVE", 4) || uint64_t(u32(wav+4))+8 != length) return 0;
    uint32_t rate = 0;
    uint16_t channels = 0, bits = 0, format = 0;
    const uint8_t *pcm = nullptr;
    size_t bytes = 0;
    size_t offset = 12;
    for (; offset + 8 <= length;)
    {
        const auto size = u32(wav + offset + 4);
        const auto *chunk = wav + offset + 8;
        if (size > length - offset - 8) return false;
        if (!std::memcmp(wav + offset, "fmt ", 4) && size >= 16)
        {
            format = u16(chunk); channels = u16(chunk + 2); rate = u32(chunk + 4); bits = u16(chunk + 14);
            if (format != 1 || (channels != 1 && channels != 2) || bits != 16 || u16(chunk+12) != channels*2 || u32(chunk+8) != rate*channels*2) return 0;
        }
        if (!std::memcmp(wav + offset, "data", 4)) { pcm = chunk; bytes = size; }
        const size_t advance = size_t(size) + 8 + (size & 1);
        if (advance > length - offset) return 0;
        offset += advance;
    }
    if (offset != length || format != 1 || bits != 16 || (channels != 1 && channels != 2) || rate < 8000 || rate > 96000 || !pcm || !bytes || bytes % (channels * 2)) return 0;
    auto *slot = freeVoice();
    if (!slot || (id && find(id))) return 0;
    Voice voice;
    voice.owner = wav; voice.pcm = pcm; voice.channels = channels; voice.frames = bytes / (channels * 2);
    voice.step = uint32_t((uint64_t(rate) << 16) / outputRate); voice.loop = loop;
    voice.id = id ? id : nextId(); voice.channel = channel;
    voice.volume = std::min<uint8_t>(100,volume);
    *slot = voice;
    return voice.id;
}

void AudioMixer::configure(const AudioPreferences &value)
{
    preferences = value;
    preferences.music.volume = std::min<uint8_t>(100, preferences.music.volume);
    preferences.effects.volume = std::min<uint8_t>(100, preferences.effects.volume);
    if (!preferences.effects.enabled || !preferences.effects.volume)
        for (auto &voice : voices)
            if (voice.channel == AudioChannel::Effects && !voice.loop) voice = Voice{};
}

AudioSourceId AudioMixer::tone(uint16_t frequency, uint16_t durationMs, AudioSourceId id, uint8_t volume, AudioChannel channel)
{
    auto *voice = freeVoice();
    if (!outputRate || !voice || !durationMs || (id && find(id))) return 0;
    *voice = Voice{};
    voice->id = id ? id : nextId(); voice->volume = std::min<uint8_t>(100,volume);
    voice->channel = channel == AudioChannel::All ? AudioChannel::Effects : channel;
    voice->toneRemaining = uint32_t(uint64_t(outputRate) * durationMs / 1000);
    voice->toneStep = uint32_t((uint64_t(std::min<uint32_t>(frequency, outputRate / 2)) << 32) / outputRate);
    return voice->id;
}

void AudioMixer::stop(AudioChannel channel)
{
    for (auto &voice : voices)
        if (channel == AudioChannel::All || voice.channel == channel) voice = Voice{};
}

AudioMixer::Voice *AudioMixer::find(AudioSourceId id)
{
    if (id) for (auto &voice : voices) if (voice.id == id) return &voice;
    return nullptr;
}
AudioMixer::Voice *AudioMixer::freeVoice() { for (auto &voice : voices) if (!voice.id) return &voice; return nullptr; }
AudioSourceId AudioMixer::nextId() { do { ++generation; } while (!generation || find(generation)); return generation; }
bool AudioMixer::stopSource(AudioSourceId id) { if (auto *voice = find(id)) { *voice = Voice{}; return true; } return false; }
bool AudioMixer::setSourceVolume(AudioSourceId id, uint8_t volume) { if (auto *voice = find(id)) { voice->volume = std::min<uint8_t>(100,volume); return true; } return false; }
bool AudioMixer::isPlaying(AudioSourceId id) const { if (id) for (const auto &voice : voices) if (voice.id == id) return true; return false; }
std::array<AudioSourceId,AUDIO_MAX_SOURCES> AudioMixer::sourceIds() const
{
    std::array<AudioSourceId,AUDIO_MAX_SOURCES> result{};
    for (size_t i=0;i<voices.size();++i) result[i] = voices[i].id;
    return result;
}
bool AudioMixer::references(const uint8_t *data) const { if (data) for (const auto &voice : voices) if (voice.owner == data) return true; return false; }
bool AudioMixer::audible(const Voice &voice, const AudioChannelPreferences &p) const { return voice.id && p.enabled && p.volume; }
bool AudioMixer::available()
{
    for (const auto &voice : voices)
        if (audible(voice, voice.channel == AudioChannel::Music ? preferences.music : preferences.effects)) return true;
    return false;
}

int32_t AudioMixer::sample(Voice &voice, const AudioChannelPreferences &p)
{
    if (!audible(voice, p)) return 0;
    const unsigned gain = unsigned(voice.volume) * p.volume;
    if (voice.toneRemaining)
    {
        const int32_t value = int32_t(std::sin(double(voice.tonePhase) * 6.283185307179586 / 4294967296.0) * 16000);
        voice.tonePhase += voice.toneStep;
        if (!--voice.toneRemaining) voice = Voice{};
        return value * int32_t(gain) / 10000;
    }
    const size_t frame = size_t(voice.position >> 16);
    const uint8_t *ptr = voice.pcm + frame * voice.channels * 2;
    int32_t value = int16_t(u16(ptr));
    if (voice.channels == 2) value = (value + int16_t(u16(ptr + 2))) / 2;
    voice.position += voice.step;
    if ((voice.position >> 16) >= voice.frames)
    {
        if (voice.loop) voice.position %= uint64_t(voice.frames) << 16;
        else voice = Voice{};
    }
    return value * int32_t(gain) / 10000;
}

int AudioMixer::read(uint8_t *buffer, size_t length)
{
    size_t written = 0;
    while (written + 2 <= length && available())
    {
        int32_t value = 0;
        for (auto &voice : voices)
            value += sample(voice,voice.channel == AudioChannel::Music ? preferences.music : preferences.effects);
        const auto mixed = int16_t(std::max<int32_t>(-32768, std::min<int32_t>(32767, value)));
        buffer[written++] = uint8_t(mixed);
        buffer[written++] = uint8_t(uint16_t(mixed) >> 8);
    }
    return int(written);
}
