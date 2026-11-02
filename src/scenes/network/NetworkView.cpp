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
    brandImg = BrandHeader::create(layout.top, 100, "Rete e connessioni");
    container = Container::create(layout.body, LV_PCT(100), LV_SIZE_CONTENT, LV_ALIGN_TOP_MID);
    Container::setFlex(container);
    lv_obj_set_style_pad_row(container, 8, 0);
#if WIFI_ENABLED
    statusCard = Container::create(container, LV_PCT(100), LV_SIZE_CONTENT);
    UITheme::card(statusCard);
    Container::setFlex(statusCard, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(statusCard, 7, 0);
    auto *title = Label::create(statusCard, LV_SYMBOL_WIFI " WI-FI", &lv_font_montserrat_12);
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
    portalLabel = Label::create(portalCard, "", &lv_font_montserrat_12);
    UITheme::muted(portalLabel);
    btnPortal = Button::create(portalCard, "Collega un dispositivo", onPortalClicked, this,
                               LV_PCT(100), 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_WIFI);
    lv_obj_add_flag(portalCard, LV_OBJ_FLAG_HIDDEN);
    qrOverlay = lv_obj_create(screen);
    lv_obj_set_size(qrOverlay, LV_PCT(100), LV_PCT(100));
    lv_obj_center(qrOverlay);
    UITheme::screen(qrOverlay);
    lv_obj_clear_flag(qrOverlay, LV_OBJ_FLAG_SCROLLABLE);
    auto *heading = Label::create(qrOverlay, "Connetti alla console", &lv_font_montserrat_16);
    lv_obj_align(heading, LV_ALIGN_TOP_LEFT, 0, 0);
    Button::create(qrOverlay, nullptr, onPortalClicked, this, 44, 44, LV_ALIGN_TOP_RIGHT, 8, -10, LV_SYMBOL_CLOSE);
    qrCode = lv_qrcode_create(qrOverlay);
    lv_qrcode_set_size(qrCode, 116);
    lv_qrcode_set_dark_color(qrCode, lv_color_hex(0x25342F));
    lv_qrcode_set_light_color(qrCode, lv_color_white());
    lv_qrcode_set_quiet_zone(qrCode, true);
    lv_obj_align(qrCode, LV_ALIGN_CENTER, 0, 0);
    qrHelp = Label::create(qrOverlay, "Inquadra il codice con il telefono", &lv_font_montserrat_12,
                           LV_TEXT_ALIGN_CENTER, LV_PCT(100));
    UITheme::muted(qrHelp);
    lv_obj_align(qrHelp, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(qrOverlay, LV_OBJ_FLAG_HIDDEN);
#endif
    btnBack = BackButton::create(layout.footer, onBackClicked, this);
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
}
void NetworkView::setPortalInfo(const std::string &url, const std::string &data)
{
    if (url == portalUrl && data == qrData) return;
    portalUrl = url; qrData = data;
    const bool available = !portalUrl.empty() && !qrData.empty();
    if (portalCard) lv_obj_set_flag(portalCard, LV_OBJ_FLAG_HIDDEN, !available);
    Label::setText(portalLabel, portalUrl.c_str());
    if (available && qrCode) lv_qrcode_set_data(qrCode, qrData.c_str());
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
