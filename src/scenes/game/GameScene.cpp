#include "GameScene.h"

#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "GameAction.h"
#include "core/SystemEventBus.h"
#include "config/Assets.h"
#include "systems/audio/AudioEvent.h"

void GameScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering GameScene");
    view.setEventHandler(this);
    view.create();
    view.showLoading();
    view.setLoadingMessage("Starting game...");
    view.setTime(remainingSec);
    view.setRunning(false);

    luaSystem = static_cast<LuaSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_LUA));
    inputSystem = static_cast<InputSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));

    if (!luaSystem)
    {
        loadState = GameLoadState::LoadFailed;
        view.showFailure();
        Log::error(Log::GAME_TAG, "LuaSystem missing");
        return;
    }

    if (!inputSystem)
    {
        loadState = GameLoadState::LoadFailed;
        view.showFailure();
        Log::error(Log::GAME_TAG, "InputSystem missing");
        return;
    }

    if (gameInfo.id.empty())
    {
        loadState = GameLoadState::LoadFailed;
        view.showFailure();
        Log::error(Log::GAME_TAG, "Game info missing");
        return;
    }

    loadState = GameLoadState::Loading;
    loadGameScript();
}

void GameScene::onUpdate()
{
    if (state == GameState::Running && remainingSec > 0)
    {
        uint32_t now = lv_tick_get();
        if (now - lastTickMs >= 1000)
        {
            lastTickMs += 1000;
            remainingSec--;
            statistics.elapsedSeconds++;
            view.setTime(remainingSec);
            if (remainingSec == 0)
            {
                state = GameState::Finished;
                luaSystem->pause();
                view.setRunning(false);
                view.showFailure();
            }
        }
    }

    view.update();
}

void GameScene::onExit()
{
    view.destroy();

    if (luaSystem)
        luaSystem->unloadScript();
}

void GameScene::completeGame()
{
    if (state == GameState::Finished)
        return;

    state = GameState::Finished;
    luaSystem->pause();
    view.setRunning(false);
    view.setVictoryStats(getVictoryStats());
    view.showVictory();
    sendAudioEvent(AUDIO_VICTORY_FILE, AudioEventType::PLAY_WAV);
}

SceneRequest GameScene::getRequest() const
{
    return request;
}

bool GameScene::handleUIEvent(const UIEvent &event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto &clicked = static_cast<const ButtonClickedEvent &>(event);

    /* MainSubview buttons */
    if (clicked.buttonId == "start_pause")
    {
        if (state == GameState::Ready || state == GameState::Paused)
        {
            state = GameState::Running;
            lastTickMs = lv_tick_get();
            luaSystem->resume();
            view.setRunning(true);
            resumeTimer();
        }
        else if (state == GameState::Running)
        {
            state = GameState::Paused;
            luaSystem->pause();
            view.setRunning(false);
            pauseTimer();
        }
        return true;
    }
    if (clicked.buttonId == "cancel")
    {
        stopGame();
        request = SceneRequest(SceneChange::GoToGameList, SceneTransition::SlideRight);
        return true;
    }
    if (clicked.buttonId == "objects")
    {
        view.showGameObject();
        return true;
    }
    if (clicked.buttonId == "back_main")
    {
        view.showMain();
        return true;
    }

    /* VictorySubview */
    if (clicked.buttonId == "complete")
    {
        request = SceneRequest(SceneChange::GoToGameList, SceneTransition::Fade);
        return true;
    }

    /* FailSubview */
    if (clicked.buttonId == "retry")
    {
        loadGameScript();
        return true;
    }
    if (clicked.buttonId == "back")
    {
        stopGame();
        request = SceneRequest(SceneChange::GoToGameList, SceneTransition::SlideRight);
        return true;
    }
    return false;
}

bool GameScene::loadGameScript()
{
    if (!luaSystem || gameInfo.id.empty())
        return false;

    view.showLoading();
    view.setLoadingMessage("Initializing input...");

    if (!inputSystem->begin(gameInfo.inputConfig))
    {
        loadState = GameLoadState::LoadFailed;
        view.showFailure();
        return false;
    }

    view.setLoadingMessage("Loading game script...");
    std::string scriptPath = getGameDataDirectory() + "/game.lua";
    remainingSec = gameInfo.durationMin * 60;
    view.setTime(remainingSec);
    luaSystem->setGameScene(this);

    if (!luaSystem->loadScript(scriptPath.c_str()))
    {
        loadState = GameLoadState::LoadFailed;
        view.showFailure();
        return false;
    }

    luaSystem->pause();
    loadState = GameLoadState::LoadComplete;
    state = GameState::Ready;
    view.showMain();
    Log::info(Log::GAME_TAG, "Game loaded");
    return true;
}

void GameScene::stopGame()
{
    if (state == GameState::Running || state == GameState::Paused)
    {
        state = GameState::Finished;
        view.setRunning(false);
    }

    if (luaSystem)
        luaSystem->unloadScript();
}

void GameScene::startTimer()
{
    statistics = GameStatistics();
    statistics.startMs = millis();
}

void GameScene::pauseTimer()
{
    if (statistics.paused)
        return;
    statistics.paused = true;
    statistics.pauseStartMs = millis();
}

void GameScene::resumeTimer()
{
    if (!statistics.paused)
    {
        if (statistics.startMs == 0)
            startTimer();
        return;
    }
    uint32_t now = millis();
    statistics.pausedSeconds += (now - statistics.pauseStartMs) / 1000;
    statistics.paused = false;
}

VictorySubview::GameStats GameScene::getVictoryStats() const
{
    VictorySubview::GameStats stats;
    stats.elapsedSeconds = statistics.elapsedSeconds;
    stats.pausedSeconds = statistics.pausedSeconds;
    stats.hintsUsed = statistics.hintsUsed;
    stats.puzzlesSolved = statistics.puzzlesSolved;
    return stats;
}