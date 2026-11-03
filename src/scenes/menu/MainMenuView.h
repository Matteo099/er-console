#pragma once

#include "MainMenuAction.h"
#include "systems/ui/View.h"
#include <lvgl.h>
#include "systems/ui/components/StatusBadge.h"

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
    enum class WiFiState { Disconnected, Connecting, Connected, AccessPoint, Error };
    enum class StorageState { Unavailable, Ready, Degraded, Error };
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; }
    void setStorageState(StorageState state);
    void setWiFiState(WiFiState state);

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *brandImg = nullptr;
    lv_obj_t *menuContainer = nullptr;

    lv_obj_t *btnNewGame = nullptr;
    lv_obj_t *btnSettings = nullptr;
    lv_obj_t *btnAbout = nullptr;
    StatusBadge::Parts sdBadge;
    StatusBadge::Parts wifiBadge;

    MainMenuAction pendingAction = MainMenuAction::None;

    static void onButtonClicked(lv_event_t *e);
};
