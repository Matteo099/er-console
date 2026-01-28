#pragma once

#include "systems/ui/View.h"
#include <lvgl.h>

class MainMenuView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t* getScreen() override { return screen; }

private:
    lv_obj_t* screen = nullptr;

    lv_obj_t* brandImg = nullptr;
    lv_obj_t* menuContainer = nullptr;

    lv_obj_t* btnStart = nullptr;
    lv_obj_t* btnSettings = nullptr;
    lv_obj_t* btnAbout = nullptr;

    static void onButtonClicked(lv_event_t* e);
};
