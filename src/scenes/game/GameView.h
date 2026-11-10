#pragma once

#include "systems/ui/View.h"
#include "GameSubview.h"
#include "subviews/MainSubview.h"
#include "subviews/LoadingSubview.h"
#include "subviews/VictorySubview.h"
#include "subviews/FailSubview.h"
#include "subviews/GameObjectSubview.h"
#include "subviews/InGameSettingsSubview.h"
#include "systems/ui/components/ConfirmationDialog.h"
#include "systems/ui/components/PlayerSelectorDialog.h"

#include <string>

class GameView : public View
{
public:

    /* View lifecycle */
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override
    {
        return screen;
    }

    /* State management */
    void showMain();
    void showLoading();
    void showVictory();
    void showFailure();
    void showGameObject();
    void showLoadError(const char *message);
    void setTitle(const std::string &title);
    void showExitConfirmation();
    void hideExitConfirmation();
    void showPlayers(uint16_t initial) { lv_obj_add_state(settingsButton,LV_STATE_DISABLED); playerSelector.show(initial); }
    void hidePlayers() { playerSelector.hide(); lv_obj_remove_state(settingsButton,LV_STATE_DISABLED); }
    bool playersVisible() const { return playerSelector.visible(); }
    void showSettings(const AudioPreferences &preferences, Text context);
    void hideSettings() { settingsSubview.hide(); }
    bool settingsVisible() const { return settingsSubview.visible(); }
    void setSettingsAudio(const AudioPreferences &value) { settingsSubview.set(value); }
    void setSettingsStatus(Text value) { settingsSubview.setStatus(value); }
    void translateSettings() { settingsSubview.translate(); }
    GameSubview currentSubview() const { return currentState; }

    void setLoadingMessage(const std::string &message);
    void setVictoryStats(const VictorySubview::GameStats &stats);
    void setGameObjectData(const std::vector<GameObjectSubview::ObjectData> &data);

    /* Main game API */
    void setTime(uint32_t seconds);
    void setRunning(bool running);


private:
    void setState(GameSubview state);

    lv_obj_t *screen = nullptr;
    lv_obj_t *titleLabel = nullptr;
    lv_obj_t *settingsButton = nullptr;
    static void onSettings(lv_event_t *event);
    ConfirmationDialog exitDialog;
    PlayerSelectorDialog playerSelector;

    GameSubview currentState = GameSubview::Main;

    MainSubview mainSubview;
    LoadingSubview loadingSubview;
    VictorySubview victorySubview;
    FailSubview failSubview;
    GameObjectSubview gameObjectSubview;
    InGameSettingsSubview settingsSubview;
};
