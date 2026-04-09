#include "GameScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "GameAction.h"

#include "core/SystemEventBus.h"
#include "log/Logger.h"

void GameScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering GameScene");
    view.setEventHandler(this);
    view.setTime(remainingSec);
    view.setRunning(false);
    view.showLoadingBar();
    view.create();

    // Get LuaSystem from SystemEventBus
    luaSystem = static_cast<LuaSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_LUA));

    if (luaSystem && !gameId.empty())
    {
        loadState = GameLoadState::Loading;
        loadGameScript();
    }
    else
    {
        loadState = GameLoadState::LoadFailed;
        view.hideLoadingBar();
        Log::error(Log::GAME_TAG, "GameScene: LuaSystem or gameId not available");
    }
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
                state = GameState::Finished;
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

SceneRequest GameScene::getRequest() const
{
    return request;
}

bool GameScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "start_pause")
    {
        if (state == GameState::Ready || state == GameState::Paused)
        {
            state = GameState::Running;
            lastTickMs = lv_tick_get();
            view.setRunning(true);
        }
        else if (state == GameState::Running)
        {
            state = GameState::Paused;
            view.setRunning(false);
        }
        return true;
    }

    if (clicked.buttonId == "cancel")
    {
        request = SceneRequest(SceneChange::GoToGameList);
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
    if (!luaSystem || gameId.empty())
        return false;

    // Build path: /games/{gameId}/game.lua
    std::string scriptPath = "/games/" + gameId + "/game.lua";

    Log::info(Log::LUA_TAG, "Loading game script: %s", scriptPath.c_str());

    if (!luaSystem->loadScript(scriptPath.c_str()))
    {
        Log::error(Log::LUA_TAG, "Failed to load game script: %s", scriptPath.c_str());
        loadState = GameLoadState::LoadFailed;
        view.hideLoadingBar();
        view.setErrorMessage("Failed to load game script");
        return false;
    }

    loadState = GameLoadState::LoadComplete;
    view.hideLoadingBar();
    Log::info(Log::LUA_TAG, "Game script loaded successfully");

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
