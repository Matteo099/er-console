#include "NetworkView.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"
#include "systems/ui/localization/I18n.h"
#include "config/Console.h"

void NetworkView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen);
    brandImg = BrandHeader::create(layout.top, 100, tr(Text::Connections));
    container = Container::create(layout.body, LV_PCT(100), LV_SIZE_CONTENT, LV_ALIGN_TOP_MID);
    Container::setFlex(container); lv_obj_set_style_pad_row(container, 8, 0);
    statusCard = Container::create(container, LV_PCT(100), LV_SIZE_CONTENT);
    UITheme::card(statusCard); Container::setFlex(statusCard);
    statusLabel = Label::create(statusCard, tr(Text::Waiting), &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    ssidLabel = Label::create(statusCard, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    ipLabel = Label::create(statusCard, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    signalLabel = Label::create(statusCard, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    auto *form = Container::create(container, LV_PCT(100), LV_SIZE_CONTENT);
    UITheme::card(form); Container::setFlex(form); lv_obj_set_style_pad_row(form, 6, 0);
    Label::create(form, tr(Text::Ssid), &lv_font_montserrat_12);
    ssidButton = Button::create(form, "", onEdit, this, LV_PCT(100), 44);
    Label::create(form, tr(Text::Password), &lv_font_montserrat_12);
    passwordButton = Button::create(form, tr(Text::EnterPassword), onEdit, this, LV_PCT(100), 44);
    lv_obj_set_height(lv_obj_get_child(ssidButton, 0), lv_font_montserrat_14.line_height);
    lv_obj_set_height(lv_obj_get_child(passwordButton, 0), lv_font_montserrat_14.line_height);
    saveButton = Button::create(form, tr(WIFI_ENABLED ? Text::SaveConnect : Text::Save), onSave, this, LV_PCT(100), 44);
    disconnectButton = Button::create(form, tr(Text::Disconnect), onDisconnect, this, LV_PCT(100), 44);
    UITheme::secondary(disconnectButton);
    lv_obj_set_state(disconnectButton, LV_STATE_DISABLED, !WIFI_ENABLED);
    messageLabel = Label::create(form, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    editor.create(screen, [this](const std::string &value)
    {
        if (editingPassword) password = value; else selectedSsid = value;
        setCredentials(selectedSsid, password);
    });
#if WIFI_ENABLED && PORTAL_ENABLED
    portalCard = Container::create(container, LV_PCT(100), LV_SIZE_CONTENT);
    UITheme::card(portalCard);
    Container::setFlex(portalCard);
    auto *portalTitle = Label::create(portalCard, tr(Text::ConsoleNetwork), UITheme::heading(18));
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
    auto *heading = Label::create(modal.top, tr(Text::ConnectPhone), UITheme::heading());
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
    qrHelp = Label::create(modal.footer, tr(Text::QrHint), &lv_font_montserrat_12,
                           LV_TEXT_ALIGN_CENTER, LV_PCT(80));
    UITheme::muted(qrHelp);
    lv_obj_align(qrHelp, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_flag(qrOverlay, LV_OBJ_FLAG_HIDDEN);
#endif
    btnBack = BackButton::create(layout.footer, onBackClicked, this);
#if WIFI_ENABLED && PORTAL_ENABLED
    Container::setFlex(layout.footer, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(layout.footer, 8, 0);
    btnPortal = Button::create(layout.footer, tr(Text::QrWifi), onPortalClicked, this, 156, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_WIFI);
    lv_obj_add_flag(btnPortal, LV_OBJ_FLAG_HIDDEN);
#endif
}
void NetworkView::update() {}
void NetworkView::destroy()
{
    editor.clear();
    screen = brandImg = container = statusCard = statusLabel = ssidLabel = ipLabel = signalLabel = nullptr;
    portalLabel = portalCard = qrCode = qrHelp = btnPortal = btnBack = qrOverlay = nullptr;
    ssidButton = passwordButton = saveButton = disconnectButton = messageLabel = nullptr;
    portalUrl.clear(); qrData.clear(); selectedSsid.clear(); password.clear(); qrVisible = false;
}
void NetworkView::setCredentials(const std::string &ssid, const std::string &secret)
{
    selectedSsid = ssid; password = secret;
    Label::setText(lv_obj_get_child(ssidButton, 0), ssid.empty() ? tr(Text::NoNetwork) : ssid.c_str());
    Label::setText(lv_obj_get_child(passwordButton, 0), secret.empty() ? tr(Text::EnterPassword) : "********");
}
void NetworkView::setStatus(const std::string &status, const std::string &ssid, const std::string &ip, int signal)
{
    Label::setText(statusLabel, status.c_str());
    Label::setText(ssidLabel, ssid.empty() ? tr(Text::NoNetwork) : ssid.c_str());
    Label::setText(ipLabel, ip.c_str()); lv_obj_set_flag(ipLabel, LV_OBJ_FLAG_HIDDEN, ip.empty());
    Label::setText(signalLabel, I18n::format(Text::Signal, signal).c_str());
    lv_obj_set_flag(signalLabel, LV_OBJ_FLAG_HIDDEN, ip.empty());
}
void NetworkView::setMessage(Text text) { Label::setText(messageLabel, tr(text)); }
void NetworkView::onEdit(lv_event_t *e)
{
    auto *v = static_cast<NetworkView *>(lv_event_get_user_data(e));
    v->editingPassword = lv_event_get_target(e) == v->passwordButton;
    v->editor.show(v->editingPassword ? tr(Text::Password) : "SSID", v->editingPassword ? v->password : v->selectedSsid, v->editingPassword);
}
void NetworkView::onSave(lv_event_t *e) { auto *v = static_cast<NetworkView *>(lv_event_get_user_data(e)); v->emitEvent(ButtonClickedEvent("save_network")); }
void NetworkView::onDisconnect(lv_event_t *e) { auto *v = static_cast<NetworkView *>(lv_event_get_user_data(e)); v->emitEvent(ButtonClickedEvent("disconnect_network")); }
void NetworkView::setPortalInfo(const std::string &url, const std::string &data)
{
    if (url == portalUrl && data == qrData && portalLocale == I18n::revision()) return;
    portalLocale = I18n::revision();
    portalUrl = url; qrData = data;
    const bool available = !portalUrl.empty() && !qrData.empty();
    if (portalCard) lv_obj_set_flag(portalCard, LV_OBJ_FLAG_HIDDEN, !available);
    if (btnPortal) lv_obj_set_flag(btnPortal, LV_OBJ_FLAG_HIDDEN, !available);
    Label::setText(portalLabel, portalUrl.c_str());
    if (available && qrCode)
    {
        const bool rendered = lv_qrcode_update(qrCode, qrData.data(), qrData.size()) == LV_RESULT_OK;
        lv_obj_set_flag(qrCode, LV_OBJ_FLAG_HIDDEN, !rendered);
        const std::string help = rendered ? std::string(tr(Text::ScanThenOpen)) + portalUrl : std::string(tr(Text::QrUnavailable)) + portalUrl;
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
