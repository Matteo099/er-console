#include "NetworkView.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/Align.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/Screen.h"

void NetworkView::create()
{
    Log::debug(Log::UI_TAG, "Creating NetworkView");

    screen = Screen::create();
    brandImg = BrandHeader::create(screen);

    container = Container::create(screen, 240, 160, LV_ALIGN_CENTER, 0, 6);
    Container::setFlex(container);

    lv_obj_t* heading = Label::create(
        container,
        "Wi-Fi Settings",
        &lv_font_montserrat_18,
        LV_TEXT_ALIGN_CENTER,
        240);
    lv_obj_set_style_text_color(heading, lv_color_hex(0x64B5F6), 0);

    statusLabel = Label::create(
        container,
        "Status: Unknown",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        240);

    ssidLabel = Label::create(
        container,
        "SSID: —",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        240);

    ipLabel = Label::create(
        container,
        "IP: —",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        240);

    btnConnect = Button::create(container, "Connect", onConnectClicked, this, 180, 40, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_WIFI);
    btnDisconnect = Button::create(container, "Disconnect", onDisconnectClicked, this, 180, 40, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_CLOSE);
    btnBack = BackButton::create(screen, onBackClicked, this);
}

void NetworkView::update()
{
    // UI is largely static; updates come from the scene
}

void NetworkView::destroy()
{
    getObjectTracker().cleanup();
    UICleanup::deleteRoot(screen);
    screen = nullptr;
    brandImg = nullptr;
    container = nullptr;
    statusLabel = nullptr;
    ssidLabel = nullptr;
    ipLabel = nullptr;
    btnConnect = nullptr;
    btnDisconnect = nullptr;
    btnBack = nullptr;
}

void NetworkView::setStatus(const std::string &statusText, const std::string &ssid, const std::string &ip)
{
    if (statusLabel)
        lv_label_set_text(statusLabel, statusText.c_str());
    if (ssidLabel)
        lv_label_set_text(ssidLabel, ssid.empty() ? "SSID: —" : (std::string("SSID: ") + ssid).c_str());
    if (ipLabel)
        lv_label_set_text(ipLabel, ip.empty() ? "IP: —" : (std::string("IP: ") + ip).c_str());
}

void NetworkView::onConnectClicked(lv_event_t *e)
{
    auto *view = static_cast<NetworkView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("wifi_connect"));
}

void NetworkView::onDisconnectClicked(lv_event_t *e)
{
    auto *view = static_cast<NetworkView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("wifi_disconnect"));
}

void NetworkView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<NetworkView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("back"));
}
