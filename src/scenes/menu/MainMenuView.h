#pragma once

#include "MainMenuAction.h"
#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"
#include <lvgl.h>

/**
 * @brief Main menu view
 * 
 * Object Hierarchy & Cleanup Pattern:
 * - Follows Pattern 1 (Simple Root-Based Cleanup)
 * - Root object: `screen` (Screen)
 * - All children owned by screen hierarchy
 * - Cleanup: Delete screen cascades to all children
 */
class MainMenuView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; }

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
