#include "SettingsView.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"
#include "config/Console.h"

void SettingsView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen, 44, 48, false);
    brandImg = BrandHeader::create(layout.top, 100, "Impostazioni");
    container = Container::create(layout.body, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_MID);
    Container::setFlex(container);
    lv_obj_set_style_pad_row(container, 6, 0);

    auto *tabs = Container::create(container, LV_PCT(100), 44);
    Container::setFlex(tabs, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(tabs, 6, 0);
    tabAppearance = Button::create(tabs, "Aspetto", onTabClicked, this, 0, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_TINT);
    tabSystem = Button::create(tabs, "Sistema", onTabClicked, this, 0, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_SETTINGS);
    lv_obj_set_flex_grow(tabAppearance, 1);
    lv_obj_set_flex_grow(tabSystem, 1);
    UITheme::secondary(tabAppearance);
    UITheme::secondary(tabSystem);
    lv_obj_add_state(tabAppearance, LV_STATE_CHECKED);

    auto *panels = Container::create(container, LV_PCT(100), 0);
    lv_obj_set_flex_grow(panels, 1);
    themePanel = Container::create(panels, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_MID);
    Container::setFlex(themePanel, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    Container::setScrollable(themePanel);
    lv_obj_set_style_pad_row(themePanel, 3, 0);
    auto *hint = Label::create(themePanel, "Un colore per la tua console", &lv_font_montserrat_12);
    UITheme::muted(hint);
    auto *colors = Container::create(themePanel, LV_PCT(100), 44);
    Container::setFlex(colors, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(colors, 6, 0);
    for (unsigned i = 0; i < 3; ++i)
    {
        const auto id = static_cast<UIThemeId>(i);
        auto *button = Button::create(colors, UITheme::title(id), onThemeClicked, this, 0, 44);
        themeButtons[i] = button;
        lv_obj_set_flex_grow(button, 1);
        lv_obj_set_style_bg_color(button, lv_color_hex(UITheme::palette(id).accent), 0);
        lv_obj_set_style_text_font(lv_obj_get_child(button, 0), &lv_font_montserrat_12, 0);
    }
    preferenceLabel = Label::create(themePanel, "Preferenze salvate", &lv_font_montserrat_12);
    UITheme::muted(preferenceLabel);

    systemPanel = Container::create(panels, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_MID);
    Container::setFlex(systemPanel);
    lv_obj_set_flex_flow(systemPanel, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(systemPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    Container::setScrollable(systemPanel);
    lv_obj_set_style_pad_row(systemPanel, 6, 0);
    lv_obj_set_style_pad_column(systemPanel, 6, 0);
    lv_obj_add_flag(systemPanel, LV_OBJ_FLAG_HIDDEN);
    btnSound = Button::create(systemPanel, "Suoni attivi", onSoundClicked, this, LV_PCT(49), 68,
                              LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_VOLUME_MID);
    iconSound = lv_obj_get_child(btnSound, 0);
    lblSound = lv_obj_get_child(btnSound, 1);
    btnMonitor = Button::create(systemPanel, "Risorse", onMonitorClicked, this,
                                LV_PCT(49), 68, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_CHARGE);
    UITheme::secondary(btnMonitor);
#if WIFI_ENABLED
    btnWifi = Button::create(systemPanel, "Rete e connessioni", onWifiClicked, this,
                             LV_PCT(100), 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_WIFI);
    UITheme::secondary(btnWifi);
    lblWifi = Label::create(systemPanel, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(lblWifi);
#endif
    btnBack = BackButton::create(layout.footer, onBackClicked, this);
    setTheme(UITheme::current());
}

void SettingsView::setTheme(UIThemeId theme)
{
    for (unsigned i = 0; i < 3; ++i)
        if (themeButtons[i])
        {
            char text[32];
            lv_snprintf(text, sizeof(text), "%s%s", i == static_cast<unsigned>(theme) ? LV_SYMBOL_OK " " : "",
                        UITheme::title(static_cast<UIThemeId>(i)));
            Label::setText(lv_obj_get_child(themeButtons[i], 0), text);
            if (i == static_cast<unsigned>(theme)) lv_obj_add_state(themeButtons[i], LV_STATE_CHECKED);
            else lv_obj_remove_state(themeButtons[i], LV_STATE_CHECKED);
        }
}
void SettingsView::setSoundEnabled(bool enabled)
{
    Label::setText(lblSound, enabled ? "Suoni attivi" : "Suoni disattivati");
    Label::setText(iconSound, enabled ? LV_SYMBOL_VOLUME_MID : LV_SYMBOL_MUTE);
}
void SettingsView::setWiFiStatus(const std::string &status) { Label::setText(lblWifi, status.c_str()); }
void SettingsView::setPreferenceStatus(const char *text) { Label::setText(preferenceLabel, text); }
void SettingsView::update() {}
void SettingsView::destroy()
{
    screen = brandImg = container = btnWifi = btnMonitor = btnSound = lblWifi = lblSound = iconSound = btnBack = nullptr;
    themePanel = systemPanel = tabAppearance = tabSystem = preferenceLabel = nullptr;
    for (auto &button : themeButtons) button = nullptr;
}
void SettingsView::onThemeClicked(lv_event_t *e)
{
    auto *view = static_cast<SettingsView *>(lv_event_get_user_data(e));
    if (!view) return;
    for (unsigned i = 0; i < 3; ++i)
        if (lv_event_get_target(e) == view->themeButtons[i])
            view->emitEvent(ButtonClickedEvent(i == 0 ? "theme_mint" : i == 1 ? "theme_sky" : "theme_coral"));
}
void SettingsView::onTabClicked(lv_event_t *e)
{
    auto *view = static_cast<SettingsView *>(lv_event_get_user_data(e));
    if (!view) return;
    view->selectTab(lv_event_get_target(e) == view->tabAppearance ? Tab::Appearance : Tab::System);
}
void SettingsView::selectTab(Tab tab)
{
    const bool appearance = tab == Tab::Appearance;
    lv_obj_set_flag(themePanel, LV_OBJ_FLAG_HIDDEN, !appearance);
    lv_obj_set_flag(systemPanel, LV_OBJ_FLAG_HIDDEN, appearance);
    lv_obj_set_state(tabAppearance, LV_STATE_CHECKED, appearance);
    lv_obj_set_state(tabSystem, LV_STATE_CHECKED, !appearance);
}
void SettingsView::onWifiClicked(lv_event_t *e) { auto *v = static_cast<SettingsView *>(lv_event_get_user_data(e)); if (v) v->emitEvent(ButtonClickedEvent("wifi_settings")); }
void SettingsView::onSoundClicked(lv_event_t *e) { auto *v = static_cast<SettingsView *>(lv_event_get_user_data(e)); if (v) v->emitEvent(ButtonClickedEvent("toggle_sound")); }
void SettingsView::onMonitorClicked(lv_event_t *e) { auto *v = static_cast<SettingsView *>(lv_event_get_user_data(e)); if (v) v->emitEvent(ButtonClickedEvent("monitor")); }
void SettingsView::onBackClicked(lv_event_t *e) { auto *v = static_cast<SettingsView *>(lv_event_get_user_data(e)); if (v) v->emitEvent(ButtonClickedEvent("back")); }
