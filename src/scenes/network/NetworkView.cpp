#include "NetworkView.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"
#include "config/Console.h"

void NetworkView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen);
    brandImg = BrandHeader::create(layout.top, 100, "Connessioni");
    container = Container::create(layout.body, LV_PCT(100), LV_SIZE_CONTENT, LV_ALIGN_TOP_MID);
    Container::setFlex(container);
    lv_obj_set_style_pad_row(container, 8, 0);
#if WIFI_ENABLED
    statusCard = Container::create(container, LV_PCT(100), LV_SIZE_CONTENT);
    UITheme::card(statusCard);
    Container::setFlex(statusCard, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(statusCard, 7, 0);
    auto *title = Label::create(statusCard, LV_SYMBOL_WIFI " WI-FI", UITheme::heading(18));
    UITheme::accent(title);
    statusLabel = Label::create(statusCard, "In attesa", &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    ssidLabel = Label::create(statusCard, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    ipLabel = Label::create(statusCard, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(ssidLabel);
    UITheme::muted(ipLabel);
#else
    Label::create(container, "Connessioni non abilitate", &lv_font_montserrat_16);
#endif
#if WIFI_ENABLED && PORTAL_ENABLED
    portalCard = Container::create(container, LV_PCT(100), LV_SIZE_CONTENT);
    UITheme::card(portalCard);
    Container::setFlex(portalCard);
    auto *portalTitle = Label::create(portalCard, "RETE DELLA CONSOLE", UITheme::heading(18));
    UITheme::accent(portalTitle);
    portalLabel = Label::create(portalCard, "", &lv_font_montserrat_12);
    UITheme::muted(portalLabel);
    lv_obj_add_flag(portalCard, LV_OBJ_FLAG_HIDDEN);
    qrOverlay = lv_obj_create(screen);
    lv_obj_remove_style_all(qrOverlay);
    // A modal must not become a fourth row in the screen's column layout.
    lv_obj_add_flag(qrOverlay, LV_OBJ_FLAG_FLOATING);
    lv_obj_set_size(qrOverlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(qrOverlay, 0, 0);
    UITheme::screen(qrOverlay);
    lv_obj_clear_flag(qrOverlay, LV_OBJ_FLAG_SCROLLABLE);
    auto modal = SceneLayout::create(qrOverlay, 44, 44, false);
    auto *heading = Label::create(modal.top, "Collega il telefono", UITheme::heading());
    lv_obj_align(heading, LV_ALIGN_LEFT_MID, 12, 0);
    Button::create(modal.footer, nullptr, onPortalClicked, this, 44, 44, LV_ALIGN_RIGHT_MID, -8, 0, LV_SYMBOL_CLOSE);
    auto *frame = lv_obj_create(modal.body);
    lv_obj_remove_style_all(frame);
    lv_obj_set_size(frame, 140, 140);
    lv_obj_set_style_bg_color(frame, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(frame, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(frame, 8, 0);
    lv_obj_center(frame);
    qrCode = lv_qrcode_create(frame);
    lv_qrcode_set_size(qrCode, 128);
    lv_qrcode_set_dark_color(qrCode, lv_color_hex(0x25342F));
    lv_qrcode_set_light_color(qrCode, lv_color_white());
    lv_qrcode_set_quiet_zone(qrCode, true);
    lv_obj_center(qrCode);
    qrHelp = Label::create(modal.footer, "Inquadra il QR per collegarti", &lv_font_montserrat_12,
                           LV_TEXT_ALIGN_CENTER, LV_PCT(80));
    UITheme::muted(qrHelp);
    lv_obj_align(qrHelp, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_flag(qrOverlay, LV_OBJ_FLAG_HIDDEN);
#endif
    btnBack = BackButton::create(layout.footer, onBackClicked, this);
#if WIFI_ENABLED && PORTAL_ENABLED
    lv_obj_set_flex_flow(layout.footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(layout.footer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(layout.footer, 8, 0);
    btnPortal = Button::create(layout.footer, "QR Wi-Fi", onPortalClicked, this, 156, 44,
                               LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_WIFI);
    lv_obj_add_flag(btnPortal, LV_OBJ_FLAG_HIDDEN);
#endif
}
void NetworkView::update() {}
void NetworkView::destroy()
{
    screen = brandImg = container = statusCard = statusLabel = ssidLabel = ipLabel = nullptr;
    portalLabel = portalCard = qrCode = qrHelp = btnPortal = btnBack = qrOverlay = nullptr;
    portalUrl.clear(); qrData.clear(); qrVisible = false;
}
void NetworkView::setStatus(const std::string &status, const std::string &ssid, const std::string &ip)
{
    Label::setText(statusLabel, status.c_str());
    Label::setText(ssidLabel, ssid.empty() ? "Nessuna rete selezionata" : ssid.c_str());
    Label::setText(ipLabel, ip.empty() ? "" : ip.c_str());
    if (ipLabel) lv_obj_set_flag(ipLabel, LV_OBJ_FLAG_HIDDEN, ip.empty());
}
void NetworkView::setPortalInfo(const std::string &url, const std::string &data)
{
    if (url == portalUrl && data == qrData) return;
    portalUrl = url; qrData = data;
    const bool available = !portalUrl.empty() && !qrData.empty();
    if (portalCard) lv_obj_set_flag(portalCard, LV_OBJ_FLAG_HIDDEN, !available);
    if (btnPortal) lv_obj_set_flag(btnPortal, LV_OBJ_FLAG_HIDDEN, !available);
    Label::setText(portalLabel, portalUrl.c_str());
    if (available && qrCode)
    {
        const bool rendered = lv_qrcode_update(qrCode, qrData.data(), qrData.size()) == LV_RESULT_OK;
        lv_obj_set_flag(qrCode, LV_OBJ_FLAG_HIDDEN, !rendered);
        const std::string help = rendered ? "Scansiona il QR, poi apri\n" + portalUrl : "QR non disponibile. Apri\n" + portalUrl;
        Label::setText(qrHelp, help.c_str());
    }
    if (!available && qrOverlay)
    {
        lv_obj_add_flag(qrOverlay, LV_OBJ_FLAG_HIDDEN);
        qrVisible = false;
    }
}
void NetworkView::toggleQrOverlay()
{
    if (!qrOverlay || qrData.empty()) return;
    qrVisible = !qrVisible;
    lv_obj_set_flag(qrOverlay, LV_OBJ_FLAG_HIDDEN, !qrVisible);
    if (qrVisible) lv_obj_move_foreground(qrOverlay);
}
void NetworkView::onPortalClicked(lv_event_t *e)
{
    auto *view = static_cast<NetworkView *>(lv_event_get_user_data(e));
    if (view) view->emitEvent(ButtonClickedEvent("portal_qr"));
}
void NetworkView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<NetworkView *>(lv_event_get_user_data(e));
    if (view) view->emitEvent(ButtonClickedEvent("back"));
}
