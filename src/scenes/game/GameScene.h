
#include "systems/ui/Scene.h"
#include "systems/lua/LuaSystem.h"
#include "systems/input/InputSystem.h"
#include "GameView.h"
#include "GameState.h"
#include <string>
#include "GameInfo.h"
#include <atomic>
#include "core/TaskMutex.h"

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

    View &getView() override { return view; }
    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent &event) override;

    // Called by SceneManager with game ID
    void setGameInfo(const GameInfo &info) { gameInfo = info; }

    void completeGame();
    void requestCompletion() { completionPending.store(true); }
    void postGameObjects(std::vector<GameObjectSubview::ObjectData> objects);

    GameView *getGameView() { return &view; }

    std::string getGameDataDirectory() const
    {
        return std::string("/games/") + gameInfo.id;
    }

private:
    struct GameStatistics
    {
        uint32_t elapsedSeconds = 0;
        uint32_t pausedSeconds = 0;

        uint32_t hintsUsed = 0;
        uint32_t puzzlesSolved = 0;

        uint32_t startMs = 0;
        uint32_t pauseStartMs = 0;

        bool paused = false;
    };

    GameStatistics statistics;

    // Game identification
    GameInfo gameInfo;

    // View
    GameView view;

    // Lua system integration
    LuaSystem *luaSystem = nullptr;
    InputSystem *inputSystem = nullptr;
    GameLoadState loadState = GameLoadState::Idle;

    // Game state
    GameState state = GameState::Ready;
    uint32_t remainingSec = 3600;
    uint32_t lastTickMs = 0;
    bool loadRequested = false;
    uint32_t loadRequestedAt = 0;
    bool exitConfirmationOpen = false;
    bool resumeAfterConfirmation = false;
    std::atomic<bool> completionPending{false};
    TaskMutex pendingMutex;
    bool objectsPending = false;
    std::vector<GameObjectSubview::ObjectData> pendingObjects;

    SceneRequest request = SceneRequest();

    // Internal helpers
    bool loadGameScript();
    void stopGame();
    void failLoad(const char *message);

    void startTimer();
    void pauseTimer();
    void resumeTimer();

    VictorySubview::GameStats getVictoryStats() const;
};
