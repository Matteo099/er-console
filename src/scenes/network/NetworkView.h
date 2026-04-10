#pragma once

#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"
#include <string>

class NetworkView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };

    void setStatus(const std::string &statusText, const std::string &ssid, const std::string &ip);

private:
    static void onConnectClicked(lv_event_t *e);
    static void onDisconnectClicked(lv_event_t *e);
    static void onBackClicked(lv_event_t *e);

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *brandImg = nullptr;
    lv_obj_t *container = nullptr;
    lv_obj_t *statusLabel = nullptr;
    lv_obj_t *ssidLabel = nullptr;
    lv_obj_t *ipLabel = nullptr;
    lv_obj_t *btnConnect = nullptr;
    lv_obj_t *btnDisconnect = nullptr;
    lv_obj_t *btnBack = nullptr;
};
