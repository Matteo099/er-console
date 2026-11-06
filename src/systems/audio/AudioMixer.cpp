#include "AudioMixer.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace
{
    uint16_t u16(const uint8_t *p) { return uint16_t(p[0]) | uint16_t(p[1]) << 8; }
    uint32_t u32(const uint8_t *p) { return uint32_t(u16(p)) | uint32_t(u16(p + 2)) << 16; }
}

bool AudioMixer::play(const uint8_t *wav, size_t length, AudioChannel channel, bool loop)
{
    if (!wav || length < 12 || std::memcmp(wav, "RIFF", 4) || std::memcmp(wav + 8, "WAVE", 4)) return false;
    uint32_t rate = 0;
    uint16_t channels = 0, bits = 0, format = 0;
    const uint8_t *pcm = nullptr;
    size_t bytes = 0;
    for (size_t offset = 12; offset + 8 <= length;)
    {
        const auto size = u32(wav + offset + 4);
        const auto *chunk = wav + offset + 8;
        if (size > length - offset - 8) return false;
        if (!std::memcmp(wav + offset, "fmt ", 4) && size >= 16)
        {
            format = u16(chunk); channels = u16(chunk + 2); rate = u32(chunk + 4); bits = u16(chunk + 14);
        }
        if (!std::memcmp(wav + offset, "data", 4)) { pcm = chunk; bytes = size; }
        const size_t advance = size_t(size) + 8 + (size & 1);
        if (advance > length - offset) break;
        offset += advance;
    }
    if (format != 1 || bits != 16 || (channels != 1 && channels != 2) || rate < 8000 || rate > 96000 || !pcm || !bytes || bytes % (channels * 2)) return false;
    Voice voice;
    voice.owner = wav; voice.pcm = pcm; voice.channels = channels; voice.frames = bytes / (channels * 2);
    voice.step = uint32_t((uint64_t(rate) << 16) / outputRate); voice.loop = loop;
    if (channel == AudioChannel::Music) music = voice;
    else { effects = voice; toneRemaining = 0; }
    return true;
}

void AudioMixer::configure(const AudioPreferences &value)
{
    preferences = value;
    preferences.music.volume = std::min<uint8_t>(100, preferences.music.volume);
    preferences.effects.volume = std::min<uint8_t>(100, preferences.effects.volume);
    if (!preferences.effects.enabled || !preferences.effects.volume) stop(AudioChannel::Effects);
}

void AudioMixer::tone(uint16_t frequency, uint16_t durationMs)
{
    effects = Voice{};
    tonePhase = 0;
    toneRemaining = uint32_t(uint64_t(outputRate) * durationMs / 1000);
    toneStep = uint32_t((uint64_t(std::min<uint32_t>(frequency, outputRate / 2)) << 32) / outputRate);
}

void AudioMixer::stop(AudioChannel channel)
{
    if (channel != AudioChannel::Effects) music = Voice{};
    if (channel != AudioChannel::Music) { effects = Voice{}; toneRemaining = 0; }
}

bool AudioMixer::references(const uint8_t *data) const { return data && (music.owner == data || effects.owner == data); }
bool AudioMixer::audible(const Voice &voice, const AudioChannelPreferences &p) const { return voice.pcm && p.enabled && p.volume; }
bool AudioMixer::available()
{
    return audible(music, preferences.music) || audible(effects, preferences.effects) ||
           (toneRemaining && preferences.effects.enabled && preferences.effects.volume);
}

int32_t AudioMixer::sample(Voice &voice, const AudioChannelPreferences &p)
{
    if (!audible(voice, p)) return 0;
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
    return value * p.volume / 100;
}

int AudioMixer::read(uint8_t *buffer, size_t length)
{
    size_t written = 0;
    while (written + 2 <= length && available())
    {
        int32_t value = sample(music, preferences.music) + sample(effects, preferences.effects);
        if (toneRemaining && preferences.effects.enabled && preferences.effects.volume)
        {
            value += int32_t(std::sin(double(tonePhase) * 6.283185307179586 / 4294967296.0) * 16000) * preferences.effects.volume / 100;
            tonePhase += toneStep; --toneRemaining;
        }
        const auto mixed = int16_t(std::max<int32_t>(-32768, std::min<int32_t>(32767, value)));
        buffer[written++] = uint8_t(mixed);
        buffer[written++] = uint8_t(uint16_t(mixed) >> 8);
    }
    return int(written);
}
