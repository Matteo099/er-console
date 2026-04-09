#pragma once

#include "systems/ui/Scene.h"
#include "systems/lua/LuaSystem.h"
#include "GameView.h"
#include "GameState.h"
#include <string>

enum class GameLoadState
{
    Idle,
    Loading,
    LoadComplete,
    LoadFailed
};

class GameScene : public Scene
{
public:
    // Scene interface
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::SlideLeft; }
    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent& event) override;

    // Called by SceneManager with game ID
    void setGameId(const std::string& id) { gameId = id; }

private:
    // Game identification
    std::string gameId;

    // View
    GameView view;

    // Lua system integration
    LuaSystem* luaSystem = nullptr;
    GameLoadState loadState = GameLoadState::Idle;
    bool luaUpdateCalledThisFrame = false;

    // Game state
    GameState state = GameState::Ready;
    uint32_t remainingSec = 3600;
    uint32_t lastTickMs = 0;

    SceneRequest request = SceneRequest();

    // Internal helpers
    bool loadGameScript();
    void updateLua();
};
