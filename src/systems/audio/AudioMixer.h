#pragma once
#include "AudioPreferences.h"
#include "AudioSources.h"
#include "drivers/storage/IFile.h"

// Fixed polyphonic PCM/tone mixer, owned by the audio task. No allocations in read().
class AudioMixer : public IFile {
  public:
    explicit AudioMixer(uint32_t outputRate) : outputRate(outputRate) {
    }

    AudioSourceId play(const uint8_t* wav, size_t length, AudioChannel channel, bool loop,
                       AudioSourceId id = 0, uint8_t volume = 100);
    AudioSourceId tone(uint16_t frequency, uint16_t durationMs, AudioSourceId id = 0,
                       uint8_t volume = 100, AudioChannel channel = AudioChannel::Effects);
    void stop(AudioChannel channel = AudioChannel::All);
    bool stopSource(AudioSourceId id);
    bool setSourceVolume(AudioSourceId id, uint8_t volume);
    bool isPlaying(AudioSourceId id) const;
    std::array<AudioSourceId, AUDIO_MAX_SOURCES> sourceIds() const;
    void configure(const AudioPreferences& value);
    bool references(const uint8_t* data) const;
    int read(uint8_t* buffer, size_t length) override;
    bool available() override;

    bool seek(size_t pos) override {
        return pos == 0 || pos == 44;
    }

    size_t length() const override {
        return 0;
    }  // Generated PCM stream, not a disk file.

    size_t write(const uint8_t*, size_t) override {
        return 0;
    }

    bool writeLine(const char*, bool) override {
        return false;
    }

    void flush() override {
    }

    void close() override {
    }

  private:
    struct Voice {
        const uint8_t *owner = nullptr, *pcm = nullptr;
        size_t frames = 0;
        uint64_t position = 0;
        uint32_t step = 0;
        uint16_t channels = 1;
        bool loop = false;
        AudioSourceId id = 0;
        AudioChannel channel = AudioChannel::Effects;
        uint8_t volume = 100;
        uint32_t toneRemaining = 0, tonePhase = 0, toneStep = 0;
    };

    std::array<Voice, AUDIO_MAX_SOURCES> voices{};
    Voice* find(AudioSourceId id);
    Voice* freeVoice();
    AudioSourceId nextId();
    uint32_t generation = 0;
    int32_t sample(Voice& voice, const AudioChannelPreferences& preferences);
    bool audible(const Voice& voice, const AudioChannelPreferences& preferences) const;
    AudioPreferences preferences;
    uint32_t outputRate;
};
