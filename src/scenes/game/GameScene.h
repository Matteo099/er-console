
#include "systems/ui/Scene.h"
#include "systems/lua/LuaSystem.h"
#include "systems/input/InputSystem.h"
#include "GameView.h"
#include "GameState.h"
#include "GameAudioAssets.h"
#include "services/GameAssetCache.h"
#include "services/DeviceConfigRepository.h"
#include "systems/ui/VolumeFeedback.h"
#include "services/StatisticsRepository.h"
#include <string>
#include "GameInfo.h"
#include "systems/ui/localization/I18n.h"
#include <atomic>
#include "core/TaskMutex.h"

enum class GameLoadState { Idle, Loading, LoadComplete, LoadFailed, CheckingHardware };

class GameScene : public Scene {
  public:
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    static GameScene* diagnosticCurrent;

    void useHardwareForDiagnostics(const InputConfig& config) {
        diagnosticHardware = true;
        gameInfo.inputConfig = config;
    }
#endif
    // Scene interface
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;
    void onPreferencesChanged() override;

    View& getView() override {
        return view;
    }

    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent& event) override;

    // Called by SceneManager with game ID
    void setGameInfo(const GameInfo& info) {
        gameInfo = info;
    }

    void completeGame();

    void requestCompletion() {
        completionPending.store(true);
    }

    void postGameObjects(std::vector<GameObjectSubview::ObjectData> objects);

    GameView* getGameView() {
        return &view;
    }

    std::string getGameDataDirectory() const {
        return std::string("/games/") + gameInfo.id;
    }

  private:
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    bool diagnosticHardware = false;
#endif
    struct SessionDisplayStatistics {
        uint32_t elapsedSeconds = 0;
        uint32_t pausedSeconds = 0;

        uint32_t hintsUsed = 0;
        uint32_t puzzlesSolved = 0;

        uint32_t startMs = 0;
        uint32_t pauseStartMs = 0;

        bool paused = false;
    };

    SessionDisplayStatistics statistics;
    GameSessionClock sessionClock;
    uint64_t statisticsSessionId = 0;
    uint16_t selectedPlayers = 1;
    void recordOutcome(GameOutcome outcome);

    // Game identification
    GameInfo gameInfo;

    // View
    GameView view;
    GameAudioAssets audioAssets;
    GameAssetCache::Token assetToken = 0;
    class AudioSystem* audioSystem = nullptr;
    DeviceConfigRepository settingsRepository;
    AudioPreferences settingsAudio;
    VolumeFeedback settingsFeedback;
    bool settingsDirty = false;
    uint32_t settingsChangedAt = 0;
    void openSettings();
    void closeSettings();
    void saveSettings();

    // Lua system integration
    LuaSystem* luaSystem = nullptr;
    InputSystem* inputSystem = nullptr;
    GameLoadState loadState = GameLoadState::Idle;
    enum class HardwareAction { None, Load, Start, Resume };
    HardwareAction hardwareAction = HardwareAction::None;
    void checkHardware(HardwareAction action);
    void updateHardware();
    void failHardware(const HardwareStatus& status);
    bool finishGameScriptLoad();

    // Game state
    GameState state = GameState::Ready;
    uint32_t remainingSec = 3600;
    uint32_t lastTickMs = 0;
    bool loadRequested = false;
    bool audioPreparing = false;
    bool hardwarePrepared = false;
    uint32_t loadRequestedAt = 0;
    bool exitConfirmationOpen = false;
    bool resumeAfterConfirmation = false;
    std::atomic<bool> completionPending{false};
    TaskMutex pendingMutex;
    bool objectsPending = false;
    std::vector<GameObjectSubview::ObjectData> pendingObjects;

    SceneRequest request = SceneRequest();

    // Internal helpers
    bool prepareHardware();
    void stopGame();
    void failLoad(const char* message);

    void startTimer();
    void pauseTimer();
    void resumeTimer();

    VictorySubview::GameStats getVictoryStats() const;
};
