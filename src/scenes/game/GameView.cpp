#include "systems/ui/localization/I18n.h"
#include "GameView.h"

#include "systems/ui/UIEvents.h"
#include "systems/ui/components/Screen.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/BrandHeader.h"
#include "log/Logger.h"

namespace {
constexpr const char* TAG = "GameView";

const char* stateToString(GameSubview state) {
    switch (state) {
        case GameSubview::Main:
            return "Main";
        case GameSubview::Loading:
            return "Loading";
        case GameSubview::Victory:
            return "Victory";
        case GameSubview::Fail:
            return "Fail";
        case GameSubview::GameObject:
            return "GameObject";
        case GameSubview::Hardware:
            return "Hardware";
        default:
            return "Unknown";
    }
}
}  // namespace

void GameView::create() {
    Log::info(TAG, "Creating GameView");

    screen = Screen::create();

    lv_obj_set_size(screen, LV_PCT(100), LV_PCT(100));

    auto layout = SceneLayout::create(screen, 44, 0, false);
    auto* header = BrandHeader::create(layout.top, 100, tr(Text::Game));
    titleLabel = lv_obj_get_child(header, 1);
    lv_obj_set_style_pad_all(layout.body, 0, 0);
    mainSubview.create(layout.body);
    loadingSubview.create(layout.body);
    victorySubview.create(layout.body);
    failSubview.create(layout.body);
    gameObjectSubview.create(layout.body);
    settingsButton = Button::create(mainSubview.getControls(), nullptr, onSettings, this, 44, 44,
                                    LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_pad_all(settingsButton, 0, 0);
    UITheme::secondary(settingsButton);

    auto callback = [this](const char* id) {
        Log::debug(TAG, "Button clicked: %s", id);
        emitEvent(ButtonClickedEvent(id));
    };

    mainSubview.setEventCallback(callback);
    victorySubview.setEventCallback(callback);
    failSubview.setEventCallback(callback);
    gameObjectSubview.setEventCallback(callback);
    hardwareSubview.create(layout.body, callback);
    settingsSubview.create(screen, [this](const UIEvent& event) {
        emitEvent(event);
    });
    playerSelector.create(screen, [this](bool confirmed, uint16_t players) {
        emitEvent(PlayerCountSelectedEvent(confirmed, players));
    });
    exitDialog.create(screen, tr(Text::LeaveQuestion), [this](bool leave) {
        emitEvent(ButtonClickedEvent(leave ? "confirm_exit" : "continue_game"));
    });

    showLoading();

    Log::info(TAG, "GameView created");
}

void GameView::destroy() {
    Log::info(TAG, "Destroying GameView");

    mainSubview.destroy();
    loadingSubview.destroy();
    victorySubview.destroy();
    failSubview.destroy();
    gameObjectSubview.destroy();
    hardwareSubview.destroy();
    exitDialog.destroy();
    settingsSubview.clear();
    playerSelector.clear();
    titleLabel = settingsButton = nullptr;

    screen = nullptr;

    Log::info(TAG, "GameView destroyed");
}

void GameView::update() {
    switch (currentState) {
        case GameSubview::Main:
            mainSubview.update();
            break;

        case GameSubview::Loading:
        case GameSubview::Victory:
        case GameSubview::Fail:
        case GameSubview::GameObject:
        case GameSubview::Hardware:
            break;
    }
}

void GameView::setState(GameSubview state) {
    if (currentState == state) {
        Log::debug(TAG, "State already %s", stateToString(state));
        return;
    }

    Log::info(TAG, "State change: %s -> %s", stateToString(currentState), stateToString(state));

    currentState = state;
    hideSettings();
    hidePlayers();
    // Keep the same action alongside the controls of the selected subview.
    auto* controls = state == GameSubview::GameObject ? gameObjectSubview.getControls()
                                                      : mainSubview.getControls();
    if (lv_obj_get_parent(settingsButton) != controls)
        lv_obj_set_parent(settingsButton, controls);
    lv_obj_set_flag(settingsButton, LV_OBJ_FLAG_HIDDEN,
                    state != GameSubview::Main && state != GameSubview::GameObject);

    mainSubview.hide();
    loadingSubview.hide();
    victorySubview.hide();
    failSubview.hide();
    gameObjectSubview.hide();
    hardwareSubview.hide();

    switch (state) {
        case GameSubview::Main:
            mainSubview.show();
            break;

        case GameSubview::Loading:
            loadingSubview.show();
            break;

        case GameSubview::Victory:
            victorySubview.show();
            break;

        case GameSubview::Fail:
            failSubview.show();
            break;

        case GameSubview::GameObject:
            gameObjectSubview.show();
            break;
        case GameSubview::Hardware:
            break;  // HardwareSubview::show supplies the report after changing state.
    }

    Log::info("GameView", "screen hidden=%d", lv_obj_has_flag(screen, LV_OBJ_FLAG_HIDDEN));
}

void GameView::showMain() {
    Log::debug(TAG, "showMain()");
    setState(GameSubview::Main);
}

void GameView::showLoading() {
    Log::debug(TAG, "showLoading()");
    setState(GameSubview::Loading);
}

void GameView::showVictory() {
    Log::debug(TAG, "showVictory()");
    setState(GameSubview::Victory);
}

void GameView::showFailure() {
    failSubview.setMessage(tr(Text::TimeExpired), tr(Text::TimeExpiredDetail));
    Log::debug(TAG, "showFailure()");
    setState(GameSubview::Fail);
}

void GameView::showLoadError(const char* message) {
    failSubview.setMessage(tr(Text::CannotStart), message);
    setState(GameSubview::Fail);
}

void GameView::showHardwareError(const HardwareStatus& status, bool interrupted) {
    setState(GameSubview::Hardware);
    hardwareSubview.show(status, interrupted);
}

void GameView::setTitle(const std::string& title) {
    Label::setText(titleLabel, title.empty() ? tr(Text::Game) : title.c_str());
}

void GameView::showExitConfirmation() {
    lv_obj_add_state(settingsButton, LV_STATE_DISABLED);
    exitDialog.show();
}

void GameView::hideExitConfirmation() {
    lv_obj_remove_state(settingsButton, LV_STATE_DISABLED);
    exitDialog.hide();
}

void GameView::showSettings(const AudioPreferences& preferences, Text context) {
    if (currentState == GameSubview::Main || currentState == GameSubview::GameObject)
        settingsSubview.show(preferences, context);
}

void GameView::onSettings(lv_event_t* event) {
    auto* self = static_cast<GameView*>(lv_event_get_user_data(event));
    self->emitEvent(ButtonClickedEvent("game_settings"));
}

void GameView::showGameObject() {
    Log::debug(TAG, "showGameObject()");
    setState(GameSubview::GameObject);
}

void GameView::setTime(uint32_t seconds) {
    Log::debug(TAG, "Time updated: %lu", static_cast<unsigned long>(seconds));
    mainSubview.setTime(seconds);
    settingsSubview.setTime(seconds);
}

void GameView::setRunning(bool running) {
    Log::debug(TAG, "Running: %s", running ? "true" : "false");
    mainSubview.setRunning(running);
}

void GameView::setLoadingMessage(const std::string& message) {
    Log::info(TAG, "Loading message: %s", message.c_str());
    loadingSubview.setMessage(message);
}

void GameView::setVictoryStats(const VictorySubview::GameStats& stats) {
    Log::info(TAG,
              "Victory stats updated (elapsedSeconds=%d, pausedSeconds=%d, hintsUsed=%d, "
              "puzzlesSolved=%d)",
              stats.elapsedSeconds, stats.pausedSeconds, stats.hintsUsed, stats.puzzlesSolved);

    victorySubview.setStats(stats);
}

void GameView::setGameObjectData(const std::vector<GameObjectSubview::ObjectData>& data) {
    Log::info(TAG, "Received %u game objects", static_cast<unsigned>(data.size()));
    gameObjectSubview.setObjects(data);
    mainSubview.setObjectsAvailable(!data.empty());
}
