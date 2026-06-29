#pragma once

#include "systems/ui/View.h"
#include "GameSubview.h"
#include "subviews/MainSubview.h"
#include "subviews/LoadingSubview.h"
#include "subviews/VictorySubview.h"
#include "subviews/FailSubview.h"

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
    void setLoadingMessage(const std::string &message);
    void setVictoryStats(const VictorySubview::GameStats &stats);

    /* Main game API */
    void setTime(uint32_t seconds);
    void setRunning(bool running);


private:
    void setState(GameSubview state);

    lv_obj_t *screen = nullptr;

    GameSubview currentState = GameSubview::Main;

    MainSubview mainSubview;
    LoadingSubview loadingSubview;
    VictorySubview victorySubview;
    FailSubview failSubview;
};