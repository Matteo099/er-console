#pragma once

#include "systems/ui/View.h"
#include <string>

class NetworkView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };

    void setStatus(const std::string &statusText, const std::string &ssid, const std::string &ip);
    void setPortalInfo(const std::string& portalUrl, const std::string& qrData);
    void toggleQrOverlay();

private:
    static void onPortalClicked(lv_event_t *e);
    static void onBackClicked(lv_event_t *e);

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *brandImg = nullptr;
    lv_obj_t *container = nullptr;
    lv_obj_t *statusCard = nullptr;
    lv_obj_t *statusLabel = nullptr;
    lv_obj_t *ssidLabel = nullptr;
    lv_obj_t *ipLabel = nullptr;
    lv_obj_t *portalLabel = nullptr;
    lv_obj_t *portalCard = nullptr;
    lv_obj_t *qrCode = nullptr;
    lv_obj_t *qrHelp = nullptr;
    lv_obj_t *btnPortal = nullptr;
    lv_obj_t *btnBack = nullptr;
    std::string portalUrl;
    std::string qrData;
    bool qrVisible = false;
    lv_obj_t *qrOverlay = nullptr;
};
