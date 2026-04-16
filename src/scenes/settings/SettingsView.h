#pragma once

#include "SettingsAction.h"
#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"
#include <string>

class SettingsView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };
    
    void setSoundEnabled(bool enabled);
    void setWiFiStatus(const std::string& status);

private:
    lv_obj_t *screen = nullptr;
    
    lv_obj_t* brandImg = nullptr;
    lv_obj_t* container = nullptr;
    lv_obj_t* btnWifi = nullptr;
    lv_obj_t* btnMonitor = nullptr;
    lv_obj_t* btnSound = nullptr;
    lv_obj_t* lblWifi = nullptr;
    lv_obj_t* lblSound = nullptr;
    lv_obj_t* btnBack = nullptr;

    SettingsAction pendingAction = SettingsAction::None;

    static void onWifiClicked(lv_event_t* e);
    static void onMonitorClicked(lv_event_t* e);
    static void onSoundClicked(lv_event_t* e);
    static void onBackClicked(lv_event_t* e);
};
