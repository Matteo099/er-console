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

    container = Container::create(screen, 240, 200, LV_ALIGN_CENTER, 0, 6);
    Container::setFlex(container);

    statusCard = Container::create(container, 220, 92, LV_ALIGN_CENTER, 0, 0);
    Container::setFlex(statusCard);
    lv_obj_set_style_bg_opa(statusCard, LV_OPA_20, 0);
    lv_obj_set_style_bg_color(statusCard, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_border_width(statusCard, 1, 0);
    lv_obj_set_style_border_color(statusCard, lv_color_hex(0x1F2937), 0);

    lv_obj_t* heading = Label::create(
        statusCard,
        "Network status",
        &lv_font_montserrat_18,
        LV_TEXT_ALIGN_CENTER,
        220);
    lv_obj_set_style_text_color(heading, lv_color_hex(0x7DD3FC), 0);

    statusLabel = Label::create(
        statusCard,
        "Status: Unknown",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        220);

    ssidLabel = Label::create(
        statusCard,
        "SSID: —",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        220);

    ipLabel = Label::create(
        statusCard,
        "IP: —",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        220);

    portalCard = Container::create(container, 220, 88, LV_ALIGN_CENTER, 0, 0);
    Container::setFlex(portalCard);
    lv_obj_set_style_bg_opa(portalCard, LV_OPA_20, 0);
    lv_obj_set_style_bg_color(portalCard, lv_color_hex(0x111827), 0);
    lv_obj_set_style_border_width(portalCard, 1, 0);
    lv_obj_set_style_border_color(portalCard, lv_color_hex(0x1F2937), 0);

    portalLabel = Label::create(
        portalCard,
        "Scan to open setup portal",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        220);

    qrCode = lv_qrcode_create(portalCard);
    lv_qrcode_set_size(qrCode, 72);
    lv_qrcode_set_dark_color(qrCode, lv_color_hex(0x0B3D2E));
    lv_qrcode_set_light_color(qrCode, lv_color_white());
    lv_qrcode_set_quiet_zone(qrCode, true);
    lv_obj_add_flag(qrCode, LV_OBJ_FLAG_HIDDEN);

    qrHelp = Label::create(
        portalCard,
        "Use the button to show the QR code",
        &lv_font_montserrat_12,
        LV_TEXT_ALIGN_CENTER,
        220);

    btnPortal = Button::create(container, "Edit Wi-Fi / QR", onPortalClicked, this, 180, 40, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_SETTINGS);
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
    portalLabel = nullptr;
    portalCard = nullptr;
    qrCode = nullptr;
    qrHelp = nullptr;
    btnPortal = nullptr;
    btnBack = nullptr;
    portalUrl.clear();
    qrData.clear();
    qrVisible = false;
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

void NetworkView::setPortalInfo(const std::string& url, const std::string& data)
{
    portalUrl = url;
    qrData = data;

    if (portalLabel)
        lv_label_set_text(portalLabel, portalUrl.empty() ? "Portal unavailable" : portalUrl.c_str());

    if (qrCode && !qrData.empty())
        lv_qrcode_set_data(qrCode, qrData.c_str());
}

void NetworkView::toggleQrOverlay()
{
    qrVisible = !qrVisible;
    if (!qrCode)
        return;

    if (qrVisible)
        lv_obj_clear_flag(qrCode, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(qrCode, LV_OBJ_FLAG_HIDDEN);
}

void NetworkView::onPortalClicked(lv_event_t *e)
{
    auto *view = static_cast<NetworkView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("portal_qr"));
}

void NetworkView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<NetworkView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("back"));
}
