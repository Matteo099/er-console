#pragma once

#include "MainMenuAction.h"
#include "systems/ui/View.h"
#include <lvgl.h>

class MainMenuView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; }
    MainMenuAction consumeAction();

private:
    lv_obj_t *screen = nullptr;

    lv_obj_t *brandImg = nullptr;
    lv_obj_t *menuContainer = nullptr;

    lv_obj_t *btnNewGame = nullptr;
    lv_obj_t *btnSettings = nullptr;
    lv_obj_t *btnAbout = nullptr;

    MainMenuAction pendingAction = MainMenuAction::None;

    static void onButtonClicked(lv_event_t *e);
};
