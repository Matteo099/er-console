#include "systems/ui/localization/I18n.h"
#include "GameScene.h"

#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "GameAction.h"

#include "core/SystemEventBus.h"
#include "config/Assets.h"
#include <Arduino.h>
#include <algorithm>

void GameScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering GameScene: %s", gameInfo.id.c_str());
    view.setEventHandler(this);
    view.create();
    view.setTitle(gameInfo.titleFor(I18n::language() == I18n::Language::English));
    view.showLoading();
    view.setLoadingMessage(tr(Text::PrepareGame));
    remainingSec = uint32_t(gameInfo.durationMin) * 60;
    view.setTime(remainingSec);
    view.setRunning(false);
    luaSystem = static_cast<LuaSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_LUA));
    inputSystem = static_cast<InputSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));
    if (!luaSystem || !inputSystem || gameInfo.id.empty())
    {
        failLoad(tr(Text::GameUnavailable));
        return;
    }
    loadState = GameLoadState::Loading;
    loadRequested = true;
    loadRequestedAt = lv_tick_get();
}

void GameScene::postGameObjects(std::vector<GameObjectSubview::ObjectData> objects)
{
    TaskMutex::Guard lock(pendingMutex);
    pendingObjects = std::move(objects);
    objectsPending = true;
}

void GameScene::onUpdate()
{
    // Let the loading screen render and the previous scene unload before I/O.
    if (loadRequested && lv_tick_get() - loadRequestedAt >= 160)
    {
        loadRequested = false;
        loadGameScript();
    }
    std::vector<GameObjectSubview::ObjectData> objects;
    bool applyObjects = false;
    {
        TaskMutex::Guard lock(pendingMutex);
        if (objectsPending)
        {
            objects.swap(pendingObjects);
            objectsPending = false;
            applyObjects = true;
        }
    }
    if (applyObjects && loadState == GameLoadState::LoadComplete) view.setGameObjectData(objects);
    if (!exitConfirmationOpen && completionPending.exchange(false) && loadState == GameLoadState::LoadComplete)
        completeGame();
    if (loadState == GameLoadState::LoadComplete && state != GameState::Finished && !luaSystem->isScriptLoaded())
        failLoad(tr(Text::GameStopped));

    if (state == GameState::Running)
    {
        const uint32_t elapsed = (lv_tick_get() - lastTickMs) / 1000;
        if (elapsed)
        {
            lastTickMs += elapsed * 1000;
            const uint32_t played = gameInfo.durationMin ? std::min(elapsed, remainingSec) : elapsed;
            statistics.elapsedSeconds += played;
            if (gameInfo.durationMin)
            {
                remainingSec -= played;
                view.setTime(remainingSec);
                if (remainingSec == 0)
                {
                    state = GameState::Finished;
                    sendAudioEvent(AUDIO_TICKING_CLOCK_FILE, AudioEventType::STOP);
                    luaSystem->pause();
                    view.setRunning(false);
                    view.showFailure();
                }
            }
        }
    }
    view.update();
}

void GameScene::onExit()
{
    stopGame();
    view.destroy();
}

void GameScene::completeGame()
{
    if (state == GameState::Finished || loadState != GameLoadState::LoadComplete)
        return;

    exitConfirmationOpen = false;
    view.hideExitConfirmation();
    state = GameState::Finished;
    luaSystem->pause();
    view.setRunning(false);
    view.setVictoryStats(getVictoryStats());
    view.showVictory();
    sendAudioEvent(nullptr, AudioEventType::STOP);
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
        if (loadState != GameLoadState::LoadComplete || exitConfirmationOpen) return true;
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
        if (exitConfirmationOpen) return true;
        resumeAfterConfirmation = state == GameState::Running;
        if (resumeAfterConfirmation)
        {
            state = GameState::Paused;
            luaSystem->pause();
            pauseTimer();
            view.setRunning(false);
        }
        exitConfirmationOpen = true;
        view.showExitConfirmation();
        return true;
    }
    if (clicked.buttonId == "continue_game")
    {
        if (!exitConfirmationOpen) return true;
        exitConfirmationOpen = false;
        view.hideExitConfirmation();
        if (resumeAfterConfirmation)
        {
            state = GameState::Running;
            lastTickMs = lv_tick_get();
            resumeTimer();
            luaSystem->resume();
            view.setRunning(true);
        }
        return true;
    }
    if (clicked.buttonId == "confirm_exit")
    {
        if (!exitConfirmationOpen) return true;
        view.hideExitConfirmation();
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
        view.showLoading();
        view.setLoadingMessage(tr(Text::PrepareGame));
        loadState = GameLoadState::Loading;
        loadRequested = true;
        loadRequestedAt = lv_tick_get();
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
    stopGame();
    loadState = GameLoadState::Loading;
    state = GameState::Ready;
    statistics = GameStatistics{};
    view.setGameObjectData({});
    remainingSec = uint32_t(gameInfo.durationMin) * 60;
    view.setTime(remainingSec);
    view.setRunning(false);
    view.showLoading();
    view.setLoadingMessage(tr(Text::PrepareDevices));
    if (!inputSystem->begin(gameInfo.inputConfig))
    {
        failLoad(tr(Text::DevicesUnavailable));
        return false;
    }
    view.setLoadingMessage(tr(Text::ReadGame));
    const std::string path = getGameDataDirectory() + "/game.lua";
    luaSystem->setGameScene(this);
    if (!luaSystem->loadScript(path.c_str()))
    {
        const bool io = luaSystem->getErrorKind() == LuaSystem::ErrorKind::Storage;
        Log::error(Log::GAME_TAG, "Game load failed: %s", luaSystem->getLastError().c_str());
        failLoad(io ? tr(Text::GameReadError) :
                      tr(Text::GameScriptError));
        return false;
    }
    luaSystem->pause();
    loadState = GameLoadState::LoadComplete;
    state = GameState::Ready;
    view.showMain();
    Log::info(Log::GAME_TAG, "Game loaded: %s, duration=%u seconds", gameInfo.id.c_str(), unsigned(remainingSec));
    return true;
}

void GameScene::failLoad(const char *message)
{
    loadRequested = false;
    loadState = GameLoadState::LoadFailed;
    stopGame();
    if (inputSystem) inputSystem->shutdown();
    view.setRunning(false);
    view.showLoadError(message);
}

void GameScene::stopGame()
{
    loadRequested = false;
    exitConfirmationOpen = false;
    completionPending.store(false);
    state = GameState::Finished;
    if (luaSystem && luaSystem->isBoundTo(this))
    {
        luaSystem->unloadScript();
        sendAudioEvent(AUDIO_TICKING_CLOCK_FILE, AudioEventType::STOP);
    }
    TaskMutex::Guard lock(pendingMutex);
    pendingObjects.clear();
    objectsPending = false;
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
