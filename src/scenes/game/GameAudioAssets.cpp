#include "GameAudioAssets.h"
#include "systems/audio/AudioSystem.h"
#include "config/Assets.h"
#include "log/Logger.h"

namespace
{
    const char *const paths[] = {AUDIO_TICKING_CLOCK_FILE,AUDIO_GAME_OVER_FILE,AUDIO_VICTORY_FILE};
}
void GameAudioAssets::begin(AudioSystem *system, uint32_t now)
{
    release(); audio = system; startedAt = now; finished = false;
    if (audio) for (size_t i=0;i<assets.size();++i) assets[i] = audio->retainAsset(paths[i]);
}
bool GameAudioAssets::prepared(uint32_t now)
{
    if (finished || !audio) return true;
    bool pending = false, failed = false;
    for (size_t i=0;i<assets.size();++i)
    {
        if (!assets[i]) assets[i] = audio->retainAsset(paths[i]); // Retry a full queue without blocking UI.
        const auto state = audio->assetState(assets[i]);
        pending |= !assets[i] || state == AudioAssetState::Pending;
        failed |= state == AudioAssetState::Failed;
    }
    if (pending && uint32_t(now-startedAt) < 15000) return false;
    finished = true;
    Log::info(Log::GAME_TAG,"Game audio prepared in %lu ms (%s)",static_cast<unsigned long>(now-startedAt),
              pending ? "timeout; best effort" : failed ? "some assets unavailable" : "all ready");
    return true; // Missing optional audio must not prevent playing the game.
}
void GameAudioAssets::release()
{
    if (audio) for (auto id : assets) if (id) audio->releaseAsset(id);
    assets.fill(0); audio = nullptr;
}
