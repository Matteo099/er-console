#include "systems/ui/localization/I18n.h"
#include "GameScene.h"
#include "systems/update/SdResources.h"

#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "GameAction.h"

#include "core/SystemEventBus.h"
#include "config/Assets.h"
#include "systems/audio/AudioSystem.h"
#include "systems/ui/UIFeedback.h"
#include <Arduino.h>
#include <algorithm>
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
GameScene* GameScene::diagnosticCurrent = nullptr;
#endif

void GameScene::onEnter() {
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    diagnosticCurrent = this;
#endif
    Log::info(Log::SCENE_TAG, "Entering GameScene: %s", gameInfo.id.c_str());
    view.setEventHandler(this);
    view.create();
    view.setTitle(gameInfo.titleFor(I18n::language() == I18n::Language::English));
    view.showLoading();
    view.setLoadingMessage(tr(Text::PrepareGame));
    remainingSec = uint32_t(gameInfo.durationMin) * 60;
    view.setTime(remainingSec);
    view.setRunning(false);
    luaSystem =
        static_cast<LuaSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_LUA));
    inputSystem = static_cast<InputSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));
    if (!SdResources::current()) {
        failLoad(tr(Text::SdResourcesMissing));
        return;
    }
    if (!luaSystem || !inputSystem || gameInfo.id.empty()) {
        failLoad(tr(Text::GameUnavailable));
        return;
    }
    loadState = GameLoadState::Loading;
    loadRequested = true;
    loadRequestedAt = lv_tick_get();
    auto* audio = static_cast<AudioSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    audioSystem = audio;
    settingsFeedback.bind(audio);
    if (audio)
        audio->stop();  // Release menu voices before reserving the game cache.
    audioPreparing = false;
    view.setLoadingMessage(tr(Text::PrepareAssets));
}

void GameScene::postGameObjects(std::vector<GameObjectSubview::ObjectData> objects) {
    TaskMutex::Guard lock(pendingMutex);
    pendingObjects = std::move(objects);
    objectsPending = true;
}

void GameScene::onUpdate() {
    sessionClock.tick(millis());
    if (view.settingsVisible())
        settingsFeedback.update(millis());
    else
        settingsFeedback.stop();
    if (settingsDirty && uint32_t(millis() - settingsChangedAt) >= 400)
        saveSettings();
    updateHardware();
    // Let the loading screen render and the previous scene unload before I/O.
    if (loadRequested && lv_tick_get() - loadRequestedAt >= 160) {
        if (!hardwarePrepared) {
            if (hardwareAction == HardwareAction::None)
                prepareHardware();
        } else {
            auto& cache = GameAssetCache::instance();
            if (!assetToken) {
                auto* storage = static_cast<StorageSystem*>(
                    SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
                assetToken = cache.begin(storage, getGameDataDirectory());
            }
            if (cache.prepared(assetToken) && !audioPreparing) {
                auto* audio = static_cast<AudioSystem*>(
                    SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
                audioAssets.begin(audio, lv_tick_get(), cache.audioPaths(assetToken));
                audioPreparing = true;
                view.setLoadingMessage(tr(Text::PrepareAudio));
            }
            if (audioPreparing && audioAssets.prepared(lv_tick_get())) {
                loadRequested = false;
                finishGameScriptLoad();
            }
        }
    }
    std::vector<GameObjectSubview::ObjectData> objects;
    bool applyObjects = false;
    {
        TaskMutex::Guard lock(pendingMutex);
        if (objectsPending) {
            objects.swap(pendingObjects);
            objectsPending = false;
            applyObjects = true;
        }
    }
    if (applyObjects && loadState == GameLoadState::LoadComplete)
        view.setGameObjectData(objects);
    if (statisticsSessionId && !exitConfirmationOpen && completionPending.exchange(false) &&
        loadState == GameLoadState::LoadComplete)
        completeGame();
    if (loadState == GameLoadState::LoadComplete && state != GameState::Finished &&
        !luaSystem->isScriptLoaded())
        failLoad(tr(Text::GameStopped));

    if (state == GameState::Running) {
        const uint32_t elapsed = (lv_tick_get() - lastTickMs) / 1000;
        if (elapsed) {
            lastTickMs += elapsed * 1000;
            const uint32_t played =
                gameInfo.durationMin ? std::min(elapsed, remainingSec) : elapsed;
            statistics.elapsedSeconds += played;
            if (gameInfo.durationMin) {
                remainingSec -= played;
                view.setTime(remainingSec);
                if (remainingSec == 0) {
                    recordOutcome(GameOutcome::Lost);
                    closeSettings();
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

void GameScene::onExit() {
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    if (diagnosticCurrent == this)
        diagnosticCurrent = nullptr;
#endif
    closeSettings();
    stopGame();
    if (inputSystem)
        inputSystem->shutdown();
    audioAssets.release();
    GameAssetCache::instance().release(assetToken);
    assetToken = 0;
    view.destroy();
}

void GameScene::openSettings() {
    if (view.settingsVisible())
        return;
    DeviceConfig config;
    settingsRepository.load(config);
    settingsAudio = config.audio;
    settingsDirty = false;
    view.showSettings(settingsAudio, state == GameState::Paused  ? Text::GameSettingsPaused
                                     : state == GameState::Ready ? Text::GameSettingsReady
                                                                 : Text::GameSettingsRunning);
    Log::info(Log::GAME_TAG, "In-game settings opened; state=%u remaining=%lu", unsigned(state),
              static_cast<unsigned long>(remainingSec));
}

void GameScene::closeSettings() {
    settingsFeedback.stop();
    if (settingsDirty)
        saveSettings();
    if (view.settingsVisible())
        Log::info(Log::GAME_TAG, "In-game settings closed; state=%u remaining=%lu", unsigned(state),
                  static_cast<unsigned long>(remainingSec));
    view.hideSettings();
}

void GameScene::saveSettings() {
    // Merge only audio into the latest preferences. A portal/theme/language
    // update while playing must not be overwritten by an older game snapshot.
    DeviceConfig latest;
    settingsRepository.load(latest);
    latest.audio.music = settingsAudio.music;
    latest.audio.effects = settingsAudio.effects;
    const bool saved = settingsRepository.savePreferences(latest);
    settingsDirty = false;
    view.setSettingsStatus(saved ? Text::Saved : Text::Temporary);
}

void GameScene::onPreferencesChanged() {
    view.setTitle(gameInfo.titleFor(I18n::language() == I18n::Language::English));
    view.translateSettings();
    if (view.settingsVisible() && !settingsDirty) {
        DeviceConfig config;
        settingsRepository.load(config);
        settingsAudio = config.audio;
        view.setSettingsAudio(settingsAudio);
    }
}

void GameScene::completeGame() {
    if (!statisticsSessionId || state == GameState::Finished ||
        loadState != GameLoadState::LoadComplete)
        return;

    exitConfirmationOpen = false;
    closeSettings();
    recordOutcome(GameOutcome::Won);
    view.hideExitConfirmation();
    state = GameState::Finished;
    luaSystem->pause();
    view.setRunning(false);
    view.setVictoryStats(getVictoryStats());
    sendAudioEvent(nullptr, AudioEventType::STOP);
    view.showVictory();  // The subview owns its one playback request.
}

SceneRequest GameScene::getRequest() const {
    return request;
}

bool GameScene::handleUIEvent(const UIEvent& event) {
    if (event.getType() == PlayerCountSelectedEvent::TYPE) {
        if (!view.playersVisible() || state != GameState::Ready ||
            loadState != GameLoadState::LoadComplete)
            return false;
        const auto& chosen = static_cast<const PlayerCountSelectedEvent&>(event);
        view.hidePlayers();
        if (!chosen.confirmed || !chosen.players)
            return true;
        selectedPlayers = chosen.players;
        checkHardware(HardwareAction::Start);
        return true;
    }
    if (view.playersVisible())
        return false;
    if (event.getType() == AudioPreferenceChangedEvent::TYPE) {
        if (!view.settingsVisible() || loadState != GameLoadState::LoadComplete ||
            state == GameState::Finished)
            return false;
        const auto& changed = static_cast<const AudioPreferenceChangedEvent&>(event);
        auto& target =
            changed.channel == AudioChannel::Music ? settingsAudio.music : settingsAudio.effects;
        const auto previous = target;
        target = changed.value;
        if (audioSystem)
            audioSystem->configure(settingsAudio);
        UIFeedback::setSoundEnabled(settingsAudio.effects.enabled && settingsAudio.effects.volume);
        if (changed.preview && previous.volume != target.volume)
            settingsFeedback.request(changed.channel, target, millis());
        else if (!target.enabled || !target.volume)
            settingsFeedback.stop();
        settingsDirty = true;
        settingsChangedAt = millis();
        view.setSettingsStatus(Text::Applied);
        return true;
    }
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "close_game_settings") {
        closeSettings();
        return true;
    }
    if (clicked.buttonId == "game_settings") {
        if (loadState == GameLoadState::LoadComplete && state != GameState::Finished &&
            !exitConfirmationOpen)
            openSettings();
        return true;
    }
    if (view.settingsVisible())
        return false;  // Background controls cannot change the game through the overlay.

    /* MainSubview buttons */
    if (clicked.buttonId == "start_pause") {
        if (loadState != GameLoadState::LoadComplete || exitConfirmationOpen)
            return true;
        if (state == GameState::Ready) {
            view.showPlayers(selectedPlayers);
        } else if (state == GameState::Paused) {
            checkHardware(HardwareAction::Resume);
        } else if (state == GameState::Running) {
            state = GameState::Paused;
            luaSystem->pause();
            view.setRunning(false);
            pauseTimer();
        }
        return true;
    }
    if (clicked.buttonId == "cancel") {
        if (exitConfirmationOpen)
            return true;
        resumeAfterConfirmation = state == GameState::Running;
        if (resumeAfterConfirmation) {
            state = GameState::Paused;
            luaSystem->pause();
            pauseTimer();
            view.setRunning(false);
        }
        exitConfirmationOpen = true;
        view.showExitConfirmation();
        return true;
    }
    if (clicked.buttonId == "continue_game") {
        if (!exitConfirmationOpen)
            return true;
        exitConfirmationOpen = false;
        view.hideExitConfirmation();
        if (resumeAfterConfirmation) {
            checkHardware(HardwareAction::Resume);
        }
        return true;
    }
    if (clicked.buttonId == "confirm_exit") {
        if (!exitConfirmationOpen)
            return true;
        view.hideExitConfirmation();
        stopGame();
        request = SceneRequest(SceneChange::GoToGameList, SceneTransition::SlideRight);
        return true;
    }
    if (clicked.buttonId == "objects") {
        view.showGameObject();
        return true;
    }
    if (clicked.buttonId == "back_main") {
        view.showMain();
        return true;
    }

    /* VictorySubview */
    if (clicked.buttonId == "complete") {
        request = SceneRequest(SceneChange::GoToGameList, SceneTransition::Fade);
        return true;
    }

    /* FailSubview */
    if (clicked.buttonId == "retry") {
        stopGame();
        view.showLoading();
        view.setLoadingMessage(tr(Text::PrepareGame));
        loadState = GameLoadState::Loading;
        loadRequested = true;
        loadRequestedAt = lv_tick_get();
        audioPreparing = false;
        audioAssets.release();
        GameAssetCache::instance().release(assetToken);
        assetToken = 0;
        view.setLoadingMessage(tr(Text::PrepareAssets));
        return true;
    }
    if (clicked.buttonId == "back") {
        stopGame();
        request = SceneRequest(SceneChange::GoToGameList, SceneTransition::SlideRight);
        return true;
    }
    return false;
}

bool GameScene::prepareHardware() {
    stopGame();
    loadState = GameLoadState::Loading;
    state = GameState::Ready;
    statistics = SessionDisplayStatistics{};
    sessionClock = GameSessionClock{};
    view.setGameObjectData({});
    remainingSec = uint32_t(gameInfo.durationMin) * 60;
    view.setTime(remainingSec);
    view.setRunning(false);
    view.showLoading();
    view.setLoadingMessage(tr(Text::PrepareDevices));
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    if (!diagnosticHardware) {
#endif
        auto* storage = static_cast<StorageSystem*>(
            SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
        std::string configuration;
        if (!storage || !storage->readTextFile((getGameDataDirectory() + "/config.json").c_str(),
                                               configuration, true)) {
            failLoad(tr(Text::GameReadError));
            return false;
        }
        JsonDocument document;
        if (deserializeJson(document, configuration)) {
            failLoad(tr(Text::CatalogConfigError));
            return false;
        }
        gameInfo.inputConfig = InputConfig::parse(document["inputs"]);
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    }
#endif
    loadState = GameLoadState::CheckingHardware;
    hardwareAction = HardwareAction::Load;
    inputSystem->requestBegin(gameInfo.inputConfig);
    view.setLoadingMessage(tr(Text::HardwareChecking));
    return true;
}

bool GameScene::finishGameScriptLoad() {
    view.setLoadingMessage(tr(Text::ReadGame));
    const std::string path = getGameDataDirectory() + "/game.lua";
    luaSystem->setGameScene(this);
    if (!luaSystem->loadScript(path.c_str())) {
        const bool io = luaSystem->getErrorKind() == LuaSystem::ErrorKind::Storage;
        Log::error(Log::GAME_TAG, "Game load failed: %s", luaSystem->getLastError().c_str());
        failLoad(io ? tr(Text::GameReadError) : tr(Text::GameScriptError));
        return false;
    }
    luaSystem->pause();
    loadState = GameLoadState::LoadComplete;
    state = GameState::Ready;
    view.showMain();
    Log::info(Log::GAME_TAG, "Game loaded: %s, duration=%u seconds", gameInfo.id.c_str(),
              unsigned(remainingSec));
    return true;
}

void GameScene::checkHardware(HardwareAction action) {
    if (hardwareAction != HardwareAction::None)
        return;
    hardwareAction = action;
    view.showLoading();
    view.setLoadingMessage(tr(Text::HardwareChecking));
    inputSystem->requestHardwareCheck();
}

void GameScene::updateHardware() {
    if (!inputSystem ||
        (hardwareAction == HardwareAction::None && !(hardwarePrepared && loadRequested) &&
         (loadState != GameLoadState::LoadComplete || state == GameState::Finished)))
        return;
    const auto status = inputSystem->hardwareStatus();
    if (status.state == HardwareState::Fault) {
        failHardware(status);
        return;
    }
    if (hardwareAction == HardwareAction::None || status.state != HardwareState::Ready)
        return;
    const auto action = hardwareAction;
    hardwareAction = HardwareAction::None;
    if (action == HardwareAction::Load) {
        hardwarePrepared = true;
        loadState = GameLoadState::Loading;
        loadRequested = true;
        view.setLoadingMessage(tr(Text::PrepareAssets));
        return;
    }
    if (action == HardwareAction::Start) {
        statisticsSessionId = StatisticsRepository::instance().start(gameInfo.id, selectedPlayers);
        startTimer();
        Log::info(Log::GAME_TAG, "Session started: id=%llu players=%u",
                  static_cast<unsigned long long>(statisticsSessionId), unsigned(selectedPlayers));
    } else
        resumeTimer();
    state = GameState::Running;
    lastTickMs = lv_tick_get();
    view.showMain();
    view.setRunning(true);
    luaSystem->resume();
}

void GameScene::failHardware(const HardwareStatus& status) {
    const bool interrupted = statisticsSessionId != 0;
    if (interrupted)
        pauseTimer();
    closeSettings();
    view.hideExitConfirmation();
    stopGame();
    inputSystem->shutdown();
    if (audioSystem)
        audioSystem->stop();
    audioAssets.release();
    GameAssetCache::instance().release(assetToken);
    assetToken = 0;
    loadState = GameLoadState::LoadFailed;
    view.setRunning(false);
    view.showHardwareError(status, interrupted);
}

void GameScene::failLoad(const char* message) {
    closeSettings();
    loadRequested = false;
    loadState = GameLoadState::LoadFailed;
    stopGame();
    if (inputSystem)
        inputSystem->shutdown();
    view.setRunning(false);
    view.showLoadError(message);
}

void GameScene::stopGame() {
    hardwareAction = HardwareAction::None;
    hardwarePrepared = false;
    recordOutcome(GameOutcome::Abandoned);
    view.hidePlayers();
    loadRequested = false;
    exitConfirmationOpen = false;
    completionPending.store(false);
    state = GameState::Finished;
    if (luaSystem && luaSystem->isBoundTo(this)) {
        luaSystem->unloadScript();
        sendAudioEvent(AUDIO_TICKING_CLOCK_FILE, AudioEventType::STOP);
    }
    TaskMutex::Guard lock(pendingMutex);
    pendingObjects.clear();
    objectsPending = false;
}

void GameScene::startTimer() {
    statistics = SessionDisplayStatistics();
    statistics.startMs = millis();
    sessionClock.start(statistics.startMs);
}

void GameScene::pauseTimer() {
    if (statistics.paused)
        return;
    statistics.paused = true;
    statistics.pauseStartMs = millis();
    sessionClock.pause(statistics.pauseStartMs);
}

void GameScene::resumeTimer() {
    if (!statistics.paused) {
        if (!sessionClock.isStarted())
            startTimer();
        return;
    }
    uint32_t now = millis();
    statistics.pausedSeconds += (now - statistics.pauseStartMs) / 1000;
    statistics.paused = false;
    sessionClock.resume(now);
}

void GameScene::recordOutcome(GameOutcome outcome) {
    if (!statisticsSessionId)
        return;
    sessionClock.stop(millis());
    auto duration = sessionClock.elapsed(millis());
    if (outcome == GameOutcome::Lost && gameInfo.durationMin)
        duration = std::min<uint64_t>(duration, uint64_t(gameInfo.durationMin) * 60000);
    StatisticsRepository::instance().finish(statisticsSessionId, outcome, duration);
    Log::info(Log::GAME_TAG, "Session finished: id=%llu outcome=%u players=%u duration=%llu ms",
              static_cast<unsigned long long>(statisticsSessionId), unsigned(outcome),
              unsigned(selectedPlayers), static_cast<unsigned long long>(duration));
    statisticsSessionId = 0;
}

VictorySubview::GameStats GameScene::getVictoryStats() const {
    VictorySubview::GameStats stats;
    stats.elapsedSeconds = sessionClock.elapsed(millis()) / 1000;
    stats.pausedSeconds = sessionClock.paused(millis()) / 1000;
    stats.players = selectedPlayers;
    stats.hintsUsed = statistics.hintsUsed;
    stats.puzzlesSolved = statistics.puzzlesSolved;
    return stats;
}
