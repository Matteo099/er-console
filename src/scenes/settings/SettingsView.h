#pragma once

#include "SettingsAction.h"
#include "systems/ui/View.h"
#include <string>
#include "systems/ui/UITheme.h"

class SettingsView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };
    
    void setSoundEnabled(bool enabled);
    void setWiFiStatus(const std::string& status);
    void setTheme(UIThemeId theme);
    void setPreferenceStatus(const char *text);

private:
    lv_obj_t *screen = nullptr;
    
    lv_obj_t* brandImg = nullptr;
    lv_obj_t* container = nullptr;
    lv_obj_t* btnWifi = nullptr;
    lv_obj_t* btnMonitor = nullptr;
    lv_obj_t* btnSound = nullptr;
    lv_obj_t* lblWifi = nullptr;
    lv_obj_t* lblSound = nullptr;
    lv_obj_t* iconSound = nullptr;
    lv_obj_t* btnBack = nullptr;
    lv_obj_t* themePanel = nullptr;
    lv_obj_t* systemPanel = nullptr;
    lv_obj_t* themeButtons[3]{};
    lv_obj_t* tabAppearance = nullptr;
    lv_obj_t* tabSystem = nullptr;
    lv_obj_t* preferenceLabel = nullptr;

    SettingsAction pendingAction = SettingsAction::None;

    static void onWifiClicked(lv_event_t* e);
    static void onMonitorClicked(lv_event_t* e);
    static void onSoundClicked(lv_event_t* e);
    static void onBackClicked(lv_event_t* e);
    static void onThemeClicked(lv_event_t* e);
    static void onTabClicked(lv_event_t* e);
};
