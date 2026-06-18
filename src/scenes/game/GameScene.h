
#include "systems/ui/Scene.h"
#include "systems/lua/LuaSystem.h"
#include "systems/input/InputSystem.h"
#include "GameView.h"
#include "GameState.h"
#include <string>
#include "GameInfo.h"

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
    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent& event) override;

    // Called by SceneManager with game ID
    void setGameInfo(const GameInfo& info) { gameInfo = info; }

private:
    // Game identification
    GameInfo gameInfo;

    // View
    GameView view;

    // Lua system integration
    LuaSystem* luaSystem = nullptr;
    InputSystem* inputSystem = nullptr;
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
