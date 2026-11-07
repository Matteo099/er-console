#pragma once
#include "systems/audio/AudioPreloads.h"

class AudioSystem;

// One reservation per event sound, held for the lifetime of a GameScene.
class GameAudioAssets
{
public:
    GameAudioAssets() = default;
    GameAudioAssets(const GameAudioAssets &) = delete;
    GameAudioAssets &operator=(const GameAudioAssets &) = delete;
    ~GameAudioAssets() { release(); }
    void begin(AudioSystem *system, uint32_t now);
    bool prepared(uint32_t now);
    void release();
private:
    AudioSystem *audio = nullptr;
    std::array<AudioAssetId,3> assets{};
    uint32_t startedAt = 0;
    bool finished = false;
};
