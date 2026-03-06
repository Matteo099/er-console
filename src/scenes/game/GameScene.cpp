#include "GameScene.h"

#include "core/SystemEventBus.h"
#include "log/Logger.h"

void GameScene::onEnter()
{
    view.create();
    view.setTime(remainingSec);
    view.setRunning(false);
    view.showLoadingBar();

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
    view.update();

    // Handle loading state
    if (loadState == GameLoadState::Loading)
    {
        // Wait for script to load
        // In future, could show progress
        return;
    }

    if (loadState == GameLoadState::LoadFailed)
    {
        // Show error and allow user to go back
        if (view.cancelPressed)
        {
            view.cancelPressed = false;
            request = SceneRequest(SceneChange::GoToGameList);
        }
        return;
    }

    // Normal game update (loadState == LoadComplete)
    updateLua();

    /* Handle buttons */
    if (view.startPressed)
    {
        view.startPressed = false;

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
    }

    if (view.cancelPressed)
    {
        view.cancelPressed = false;
        request = SceneRequest(SceneChange::GoToGameList);
        return;
    }

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
