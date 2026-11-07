#pragma once
#include "systems/ui/View.h"
#include "systems/ui/UITheme.h"
#include "systems/ui/localization/I18n.h"
#include "AudioPreferenceSubview.h"

class SettingsView : public View
{
public:
    enum class Tab { Appearance, Audio, Language, System };
    void create() override;
    void update() override {}
    void destroy() override;
    lv_obj_t *getScreen() override { return screen; }
    void setAudio(const AudioPreferences &preferences);
    void setWiFiStatus(const std::string &status);
    void setTheme(UIThemeId theme);
    void setPreferenceStatus(Text text);
    void selectTab(Tab tab);
    Tab currentTab() const { return selectedTab; }
    void translate();
private:
    static void onTab(lv_event_t *event);
    static void onTheme(lv_event_t *event);
    static void onLanguage(lv_event_t *event);
    static void onButton(lv_event_t *event);
    lv_obj_t *screen = nullptr, *titleLabel = nullptr, *themeHint = nullptr, *languageHint = nullptr;
    lv_obj_t *preferenceLabels[4]{}, *panels[4]{}, *tabs[4]{}, *themeButtons[3]{}, *languageButtons[2]{};
    lv_obj_t *networkButton = nullptr, *resourcesButton = nullptr, *backButton = nullptr, *networkLabel = nullptr;
    Text preferenceText = Text::Saved;
    Tab selectedTab = Tab::Appearance;
    AudioPreferenceSubview music, effects;
};
