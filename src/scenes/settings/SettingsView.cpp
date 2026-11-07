#include "SettingsView.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"

void SettingsView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen, 44, 48, false);
    auto *header = BrandHeader::create(layout.top, 100, tr(Text::Settings));
    titleLabel = lv_obj_get_child(header, 1);
    auto *container = Container::create(layout.body, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_MID);
    Container::setFlex(container); lv_obj_set_style_pad_row(container, 6, 0);
    auto *tabRow = Container::create(container, LV_PCT(100), 44);
    Container::setFlex(tabRow, LV_FLEX_FLOW_ROW); lv_obj_set_style_pad_column(tabRow, 4, 0);
    const Text tabNames[] = {Text::Theme, Text::Audio, Text::Language, Text::System};
    for (unsigned i = 0; i < 4; ++i)
    {
        tabs[i] = Button::create(tabRow, tr(tabNames[i]), onTab, this, 0, 44);
        lv_obj_set_style_pad_hor(tabs[i], 4, 0);
        lv_obj_set_flex_grow(tabs[i], 1); UITheme::secondary(tabs[i]);
        Label::setFont(lv_obj_get_child(tabs[i], 0), &lv_font_montserrat_12);
    }
    auto *host = Container::create(container, LV_PCT(100), 0);
    lv_obj_set_flex_grow(host, 1);
    for (auto &panel : panels)
    {
        panel = Container::create(host, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_MID);
        Container::setFlex(panel, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        Container::setScrollable(panel); lv_obj_set_style_pad_row(panel, 6, 0);
    }
    themeHint = Label::create(panels[0], tr(Text::ThemeHint), &lv_font_montserrat_12);
    UITheme::muted(themeHint);
    auto *colors = Container::create(panels[0], LV_PCT(100), 44);
    Container::setFlex(colors, LV_FLEX_FLOW_ROW); lv_obj_set_style_pad_column(colors, 6, 0);
    for (unsigned i = 0; i < 3; ++i)
    {
        themeButtons[i] = Button::create(colors, UITheme::title(static_cast<UIThemeId>(i)), onTheme, this, 0, 44);
        lv_obj_set_flex_grow(themeButtons[i], 1);
        lv_obj_set_style_bg_color(themeButtons[i], lv_color_hex(UITheme::palette(static_cast<UIThemeId>(i)).accent), 0);
        Label::setFont(lv_obj_get_child(themeButtons[i], 0), &lv_font_montserrat_12);
    }
    music.create(panels[1], Text::Music, [this](AudioChannelPreferences value, bool preview) { emitEvent(AudioPreferenceChangedEvent(AudioChannel::Music, value, preview)); });
    effects.create(panels[1], Text::Effects, [this](AudioChannelPreferences value, bool preview) { emitEvent(AudioPreferenceChangedEvent(AudioChannel::Effects, value, preview)); });
    languageHint = Label::create(panels[2], tr(Text::LanguageHint), &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(languageHint);
    auto *languageRow = Container::create(panels[2], LV_PCT(100), 44);
    Container::setFlex(languageRow, LV_FLEX_FLOW_ROW); lv_obj_set_style_pad_column(languageRow, 6, 0);
    for (unsigned i = 0; i < 2; ++i)
    {
        languageButtons[i] = Button::create(languageRow, tr(i ? Text::English : Text::Italian), onLanguage, this, 0, 44);
        lv_obj_set_flex_grow(languageButtons[i], 1);
    }
    resourcesButton = Button::create(panels[3], tr(Text::Resources), onButton, this, LV_PCT(100), 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_CHARGE);
    networkButton = Button::create(panels[3], tr(Text::Network), onButton, this, LV_PCT(100), 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_WIFI);
    UITheme::secondary(resourcesButton); UITheme::secondary(networkButton);
    networkLabel = Label::create(panels[3], "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(networkLabel);
    for (unsigned i = 0; i < 4; ++i)
    {
        preferenceLabels[i] = Label::create(panels[i], tr(preferenceText), &lv_font_montserrat_12);
        UITheme::muted(preferenceLabels[i]);
    }
    backButton = Button::create(layout.footer, tr(Text::Back), onButton, this, 132, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_LEFT);
    UITheme::secondary(backButton);
    selectTab(selectedTab); translate();
}
void SettingsView::selectTab(Tab tab)
{
    selectedTab = tab;
    for (unsigned i = 0; i < 4; ++i)
    {
        lv_obj_set_flag(panels[i], LV_OBJ_FLAG_HIDDEN, i != unsigned(tab));
        lv_obj_set_state(tabs[i], LV_STATE_CHECKED, i == unsigned(tab));
    }
}
void SettingsView::setAudio(const AudioPreferences &value) { music.set(value.music); effects.set(value.effects); }
void SettingsView::setWiFiStatus(const std::string &status) { Label::setText(networkLabel, status.c_str()); }
void SettingsView::setTheme(UIThemeId theme)
{
    for (unsigned i = 0; i < 3; ++i)
    {
        const auto text = std::string(i == unsigned(theme) ? LV_SYMBOL_OK " " : "") + UITheme::title(static_cast<UIThemeId>(i));
        Label::setText(lv_obj_get_child(themeButtons[i], 0), text.c_str());
        lv_obj_set_state(themeButtons[i], LV_STATE_CHECKED, i == unsigned(theme));
    }
}
void SettingsView::setPreferenceStatus(Text text)
{
    preferenceText = text;
    for (auto *label : preferenceLabels) Label::setText(label, tr(text));
}
void SettingsView::translate()
{
    Label::setText(titleLabel, tr(Text::Settings));
    const Text names[] = {Text::Theme,Text::Audio,Text::Language,Text::System};
    for (unsigned i = 0; i < 4; ++i) Label::setText(lv_obj_get_child(tabs[i], 0), tr(names[i]));
    Label::setText(themeHint, tr(Text::ThemeHint)); Label::setText(languageHint, tr(Text::LanguageHint));
    Label::setText(lv_obj_get_child(resourcesButton, 1), tr(Text::Resources));
    Label::setText(lv_obj_get_child(networkButton, 1), tr(Text::Network));
    Label::setText(lv_obj_get_child(backButton, 1), tr(Text::Back));
    for (unsigned i = 0; i < 2; ++i) lv_obj_set_state(languageButtons[i], LV_STATE_CHECKED, (I18n::language() == I18n::Language::English) == bool(i));
    music.translate(); effects.translate(); setTheme(UITheme::current()); setPreferenceStatus(preferenceText);
}
void SettingsView::destroy()
{
    music.clear(); effects.clear(); screen = nullptr;
    titleLabel = themeHint = languageHint = networkButton = resourcesButton = backButton = networkLabel = nullptr;
    for (unsigned i = 0; i < 4; ++i) preferenceLabels[i] = panels[i] = tabs[i] = nullptr;
    for (auto &button : themeButtons) button = nullptr;
    for (auto &button : languageButtons) button = nullptr;
}
void SettingsView::onTab(lv_event_t *e)
{
    auto *v = static_cast<SettingsView *>(lv_event_get_user_data(e));
    for (unsigned i = 0; i < 4; ++i) if (lv_event_get_target(e) == v->tabs[i]) v->selectTab(static_cast<Tab>(i));
}
void SettingsView::onTheme(lv_event_t *e)
{
    auto *v = static_cast<SettingsView *>(lv_event_get_user_data(e));
    for (unsigned i = 0; i < 3; ++i) if (lv_event_get_target(e) == v->themeButtons[i])
        v->emitEvent(ButtonClickedEvent(i == 0 ? "theme_mint" : i == 1 ? "theme_sky" : "theme_coral"));
}
void SettingsView::onLanguage(lv_event_t *e)
{
    auto *v = static_cast<SettingsView *>(lv_event_get_user_data(e));
    v->emitEvent(ButtonClickedEvent(lv_event_get_target(e) == v->languageButtons[0] ? "language_it" : "language_en"));
}
void SettingsView::onButton(lv_event_t *e)
{
    auto *v = static_cast<SettingsView *>(lv_event_get_user_data(e));
    v->emitEvent(ButtonClickedEvent(lv_event_get_target(e) == v->backButton ? "back" : lv_event_get_target(e) == v->resourcesButton ? "monitor" : "wifi_settings"));
}
