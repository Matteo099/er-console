#include "GameScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "GameAction.h"

#include "core/SystemEventBus.h"
#include "log/Logger.h"
#include "config/Assets.h"

void GameScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering GameScene");
    view.setEventHandler(this);
    view.create();
    view.setTime(remainingSec);
    view.setRunning(false);
    view.showLoadingBar();

    // Get LuaSystem from SystemEventBus
    luaSystem = static_cast<LuaSystem *>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_LUA));
    inputSystem = static_cast<InputSystem *>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));

    if (!luaSystem)
    {
        loadState = GameLoadState::LoadFailed;
        view.hideLoadingBar();
        view.setErrorMessage("Lua system not available");
        Log::error(Log::GAME_TAG, "LuaSystem service not found in SystemEventBus");
        return;
    }

    if (!inputSystem)
    {
        loadState = GameLoadState::LoadFailed;
        view.hideLoadingBar();
        view.setErrorMessage("Input system not available");
        Log::error(Log::GAME_TAG, "InputSystem service not found in SystemEventBus");
        return;
    }

    if (gameInfo.id.empty())
    {
        loadState = GameLoadState::LoadFailed;
        view.hideLoadingBar();
        view.setErrorMessage("Game info not available");
        Log::error(Log::GAME_TAG, "Game info not available");
        return;
    }

    loadState = GameLoadState::Loading;
    loadGameScript();
}

void GameScene::onUpdate()
{
    /* Timer */
    if (state == GameState::Running && remainingSec > 0)
    {
        uint32_t now = lv_tick_get();
        if (now - lastTickMs >= 1000)
        {
            lastTickMs += 1000;
            remainingSec--;
            view.setTime(remainingSec);

            if (remainingSec == 0)
            {
                state = GameState::Finished;

                luaSystem->pause();

                view.setRunning(false);
                view.showGameOver();
                AudioEvent evt{};
                evt.type = AudioEventType::PLAY_WAV;
                strncpy(
                    evt.wav.path,
                    AUDIO_GAME_OVER_FILE,
                    AudioEvent::MAX_PATH_LENGTH - 1);
                evt.wav.path[AudioEvent::MAX_PATH_LENGTH - 1] = '\0';
                xQueueSend(
                    SystemEventBus::audioQueue,
                    &evt,
                    portMAX_DELAY);
            }
        }
    }
    view.update();
}

void GameScene::onExit()
{
    view.destroy();
    if (luaSystem)
    {
        luaSystem->unloadScript();
    }
}

void GameScene::completeGame()
{
    if (state == GameState::Finished)
        return;

    Log::info(Log::GAME_TAG, "Game completed successfully");
    state = GameState::Finished;

    luaSystem->pause();

    view.setRunning(false);
    view.showVictory();

    AudioEvent evt{};
    evt.type = AudioEventType::PLAY_WAV;
    strncpy(
        evt.wav.path,
        AUDIO_VICTORY_FILE,
        AudioEvent::MAX_PATH_LENGTH - 1);
    evt.wav.path[AudioEvent::MAX_PATH_LENGTH - 1] = '\0';
    xQueueSend(
        SystemEventBus::audioQueue,
        &evt,
        portMAX_DELAY);
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
    if (clicked.buttonId == "start_pause")
    {
        if (state == GameState::Ready || state == GameState::Paused)
        {
            state = GameState::Running;
            lastTickMs = lv_tick_get();
            luaSystem->resume();
            view.setRunning(true);
        }
        else if (state == GameState::Running)
        {
            state = GameState::Paused;
            luaSystem->pause();
            view.setRunning(false);
        }
        return true;
    }

    if (clicked.buttonId == "cancel")
    {
        stopGame();
        request = SceneRequest(SceneChange::GoToGameList, SceneTransition::SlideRight);
        return true;
    }

    if (clicked.buttonId == "info")
    {
        // TODO: implement info action
        return true;
    }

    if (clicked.buttonId == "settings")
    {
        // TODO: navigate to settings from game scene if required
        return true;
    }

    return false;
}

bool GameScene::loadGameScript()
{
    if (!luaSystem || gameInfo.id.empty())
        return false;

    // prepare input system with game-specific configuration
    Log::info(Log::GAME_TAG, "Initializing InputSystem with game input configuration");
    if (!inputSystem->begin(gameInfo.inputConfig))
    {
        loadState = GameLoadState::LoadFailed;
        view.hideLoadingBar();
        view.setErrorMessage("Failed to initialize input system");
        Log::error(Log::GAME_TAG, "Failed to initialize InputSystem with game input configuration");
        return false;
    }
    Log::info(Log::GAME_TAG, "InputSystem initialized successfully with game input configuration");

    // Build path: /games/{gameId}/game.lua
    std::string scriptPath = "/games/" + gameInfo.id + "/game.lua";
    remainingSec = gameInfo.durationMin * 60;
    view.setTime(remainingSec);

    Log::info(Log::GAME_TAG, "Loading game script: %s", scriptPath.c_str());

    if (!luaSystem->loadScript(scriptPath.c_str()))
    {
        Log::error(Log::GAME_TAG, "Failed to load game script: %s", scriptPath.c_str());
        loadState = GameLoadState::LoadFailed;
        view.hideLoadingBar();
        view.setErrorMessage("Failed to load game script");
        return false;
    }

    luaSystem->pause();
    loadState = GameLoadState::LoadComplete;
    view.hideLoadingBar();
    Log::info(Log::GAME_TAG, "Game script loaded successfully");

    return true;
}

void GameScene::updateLua()
{
    if (!luaSystem)
        return;

    // Call Lua update if game is running
    if (state == GameState::Running)
    {
        luaSystem->update();
    }
}

void GameScene::stopGame()
{
    if (state == GameState::Running || state == GameState::Paused)
    {
        state = GameState::Finished;
        view.setRunning(false);
        Log::info(Log::GAME_TAG, "Game stopped by user");
    }

    luaSystem->unloadScript();
}