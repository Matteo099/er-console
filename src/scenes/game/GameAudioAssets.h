#pragma once
#include "systems/audio/AudioPreloads.h"
#include <string>
#include <vector>

class AudioSystem;

// System event sounds and a bounded set of game WAVs, held for a GameScene.
class GameAudioAssets
{
public:
    GameAudioAssets() = default;
    GameAudioAssets(const GameAudioAssets &) = delete;
    GameAudioAssets &operator=(const GameAudioAssets &) = delete;
    ~GameAudioAssets() { release(); }
    void begin(AudioSystem *system, uint32_t now,const std::vector<std::string> &gamePaths = {});
    bool prepared(uint32_t now);
    void release();
private:
    AudioSystem *audio = nullptr;
    // Leave one audio/cache slot available for UI sounds and dynamic effects.
    std::array<AudioAssetId,AudioPreloads::CAPACITY-1> assets{};
    std::vector<std::string> paths;
    uint32_t startedAt = 0;
    bool finished = false;
};
