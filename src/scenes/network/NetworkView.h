#pragma once

#include "systems/ui/View.h"
#include <string>
#include "systems/ui/components/TextInputDialog.h"
#include "systems/ui/localization/I18n.h"
#include "WifiNetworkPicker.h"

class NetworkView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };

    void setStatus(const std::string &statusText, const std::string &ssid, const std::string &ip, int signal = 0);
    void showStatus();
    void setCredentials(const std::string &ssid, const std::string &password);
    void setMessage(Text text);
    void setScanResults(const WifiScanSnapshot &scan) { picker.setSnapshot(scan); }
    const std::string &ssid() const { return selectedSsid; }
    const std::string &secret() const { return password; }
    bool requiresPassword() const { return passwordRequired; }
    void setPortalInfo(const std::string& portalUrl, const std::string& qrData);
    void toggleQrOverlay();

private:
    static void onPortalClicked(lv_event_t *e);
    static void onBackClicked(lv_event_t *e);
    static void onEdit(lv_event_t *e);
    static void onSave(lv_event_t *e);
    static void onDisconnect(lv_event_t *e);

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
    lv_obj_t *signalLabel = nullptr, *ssidButton = nullptr, *passwordButton = nullptr;
    lv_obj_t *saveButton = nullptr, *disconnectButton = nullptr, *messageLabel = nullptr;
    TextInputDialog editor;
    WifiNetworkPicker picker;
    std::string selectedSsid, password;
    bool passwordRequired = true;
    uint32_t portalLocale = 0;
};
