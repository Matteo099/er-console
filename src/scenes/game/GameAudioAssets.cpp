#include "GameAudioAssets.h"
#include "systems/audio/AudioSystem.h"
#include "config/Assets.h"
#include "log/Logger.h"
#include <algorithm>

namespace {
const char* const corePaths[] = {AUDIO_TICKING_CLOCK_FILE, AUDIO_GAME_OVER_FILE,
                                 AUDIO_VICTORY_FILE};
}

void GameAudioAssets::begin(AudioSystem* system, uint32_t now,
                            const std::vector<std::string>& gamePaths) {
    release();
    audio = system;
    startedAt = now;
    finished = false;
    paths.assign(corePaths, corePaths + 3);
    for (const auto& path : gamePaths) {
        if (paths.size() == assets.size())
            break;
        if (std::find(paths.begin(), paths.end(), path) == paths.end())
            paths.push_back(path);
    }
    if (audio)
        for (size_t i = 0; i < paths.size(); ++i)
            assets[i] = audio->retainAsset(paths[i].c_str());
}

bool GameAudioAssets::prepared(uint32_t now) {
    if (finished || !audio)
        return true;
    bool pending = false, failed = false;
    for (size_t i = 0; i < paths.size(); ++i) {
        if (!assets[i])
            assets[i] =
                audio->retainAsset(paths[i].c_str());  // Retry a full queue without blocking UI.
        const auto state = audio->assetState(assets[i]);
        pending |= !assets[i] || state == AudioAssetState::Pending;
        failed |= state == AudioAssetState::Failed;
    }
    if (pending && uint32_t(now - startedAt) < 15000)
        return false;
    finished = true;
    Log::info(Log::GAME_TAG, "Game audio prepared in %lu ms (%s)",
              static_cast<unsigned long>(now - startedAt),
              pending  ? "timeout; best effort"
              : failed ? "some assets unavailable"
                       : "all ready");
    return true;  // Missing optional audio must not prevent playing the game.
}

void GameAudioAssets::release() {
    if (audio)
        for (auto id : assets)
            if (id)
                audio->releaseAsset(id);
    assets.fill(0);
    paths.clear();
    audio = nullptr;
}
