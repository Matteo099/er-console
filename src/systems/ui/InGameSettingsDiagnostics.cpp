#include "InGameSettingsDiagnostics.h"
#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include <cstring>
#include "scenes/game/GameView.h"
#include "systems/ui/UIFeedback.h"

namespace {
lv_obj_t* findButton(lv_obj_t* root, const char* icon) {
    if (lv_obj_check_type(root, &lv_label_class) && !std::strcmp(lv_label_get_text(root), icon) &&
        lv_obj_check_type(lv_obj_get_parent(root), &lv_button_class))
        return lv_obj_get_parent(root);
    for (unsigned i = 0; i < lv_obj_get_child_count(root); ++i)
        if (auto* found = findButton(lv_obj_get_child(root, i), icon))
            return found;
    return nullptr;
}

lv_obj_t* findSlider(lv_obj_t* root) {
    if (lv_obj_check_type(root, &lv_slider_class))
        return root;
    for (unsigned i = 0; i < lv_obj_get_child_count(root); ++i)
        if (auto* found = findSlider(lv_obj_get_child(root, i)))
            return found;
    return nullptr;
}

bool fitsFooter(lv_obj_t* button) {
    if (!button || !lv_obj_is_visible(button))
        return false;
    lv_area_t bounds;
    lv_obj_get_coords(button, &bounds);
    if (lv_obj_get_width(button) < 44 || lv_obj_get_height(button) < 44 || bounds.x1 < 0 ||
        bounds.x2 >= 320 || bounds.y1 < 160 || bounds.y2 >= 240)
        return false;
    auto* controls = lv_obj_get_parent(button);
    for (unsigned i = 0; i < lv_obj_get_child_count(controls); ++i) {
        auto* other = lv_obj_get_child(controls, i);
        if (other == button || !lv_obj_is_visible(other))
            continue;
        lv_area_t area;
        lv_obj_get_coords(other, &area);
        if (bounds.x1 <= area.x2 && area.x1 <= bounds.x2 && bounds.y1 <= area.y2 &&
            area.y1 <= bounds.y2)
            return false;
    }
    return true;
}

struct Capture : UIEventHandler {
    unsigned audioChanges = 0, settingsRequests = 0;
    AudioChannel channel = AudioChannel::All;
    AudioChannelPreferences value;
    bool preview = false;

    bool handleUIEvent(const UIEvent& event) override {
        if (event.getType() == AudioPreferenceChangedEvent::TYPE) {
            const auto& changed = static_cast<const AudioPreferenceChangedEvent&>(event);
            ++audioChanges;
            channel = changed.channel;
            value = changed.value;
            preview = changed.preview;
        } else if (event.getType() == ButtonClickedEvent::TYPE &&
                   static_cast<const ButtonClickedEvent&>(event).buttonId == "game_settings")
            ++settingsRequests;
        return true;
    }
};
}  // namespace

void runInGameSettingsDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool okay, const char* name) {
        okay ? ++passed : ++failed;
        Serial.printf("@INGAMECHECK %s %s\n", okay ? "PASS" : "FAIL", name);
    };
    const auto sound = UIFeedback::soundEnabled();
    UIFeedback::setSoundEnabled(false);
    auto* original = lv_display_get_default();
    auto* display = lv_display_create(320, 240);
    lv_display_set_default(display);
    {
        GameView view;
        Capture events;
        view.setEventHandler(&events);
        view.create();
        auto* screen = view.getScreen();
        lv_screen_load(screen);
        lv_obj_update_layout(screen);
        auto* gear = findButton(screen, LV_SYMBOL_SETTINGS);
        check(gear && !lv_obj_is_visible(gear) && !view.settingsVisible(),
              "loading screen keeps settings unavailable");
        GameObjectSubview::ObjectData object;
        object.title = "Preview";
        view.setGameObjectData({object});  // Include all four main controls in the layout check.
        view.setTime(120);
        view.setRunning(true);
        view.showMain();
        lv_obj_update_layout(screen);
        check(fitsFooter(gear),
              "game gear fits the footer alongside all other controls with a 44-pixel hit target");
        if (gear)
            lv_obj_send_event(gear, LV_EVENT_CLICKED, nullptr);
        check(events.settingsRequests == 1,
              "gear publishes an in-game action without a scene request");
        auto* cancel = findButton(screen, LV_SYMBOL_CLOSE);
        AudioPreferences preferences;
        preferences.music = {false, 35};
        preferences.effects = {true, 65};
        view.showSettings(preferences, Text::GameSettingsRunning);
        lv_obj_update_layout(screen);
        check(view.settingsVisible() && view.currentSubview() == GameSubview::Main &&
                  view.getScreen() == screen,
              "opening settings preserves the screen and active game subview");
        check(events.audioChanges == 0,
              "opening settings does not change preferences or request audio feedback");
        if (cancel) {
            lv_area_t area;
            lv_obj_get_coords(cancel, &area);
            lv_point_t point{(area.x1 + area.x2) / 2, (area.y1 + area.y2) / 2};
            auto* hit = lv_indev_search_obj(screen, &point);
            bool leaks = false;
            for (auto* object = hit; object; object = lv_obj_get_parent(object))
                leaks |= object == cancel;
            check(hit && !leaks, "opaque settings overlay intercepts input above game controls");
        } else
            check(false, "underlying cancel control exists");
        auto* slider = findSlider(screen);
        if (slider) {
            lv_slider_set_value(slider, 55, LV_ANIM_OFF);
            lv_obj_send_event(slider, LV_EVENT_VALUE_CHANGED, nullptr);
            check(events.audioChanges == 1 && events.channel == AudioChannel::Music &&
                      events.value.volume == 55 && events.preview,
                  "overlay sliders publish the same immediate audio events as Settings");
        } else
            check(false, "overlay audio controls exist");
        view.setTime(119);
        check(view.settingsVisible() && view.getScreen() == screen,
              "timer updates keep the overlay and scene alive");
        view.hideSettings();
        check(!view.settingsVisible() && view.currentSubview() == GameSubview::Main,
              "closing settings returns to the existing main subview");
        view.showGameObject();
        lv_obj_update_layout(screen);
        check(fitsFooter(gear), "object view keeps the gear in its footer without overlapping "
                                "navigation or page count");
        view.showSettings(preferences, Text::GameSettingsPaused);
        view.hideSettings();
        check(view.currentSubview() == GameSubview::GameObject && view.getScreen() == screen,
              "object view remains selected after opening and closing settings");
        view.showMain();
        lv_obj_update_layout(screen);
        check(fitsFooter(gear),
              "returning to the timer moves the same gear back beside the game controls");
        view.showSettings(preferences, Text::GameSettingsRunning);
        view.showLoading();
        check(!view.settingsVisible() && !lv_obj_is_visible(gear),
              "transition to a non-playable state dismisses settings");
        view.destroy();
    }
    lv_display_delete(display);
    lv_display_set_default(original);
    UIFeedback::setSoundEnabled(sound);
    Serial.printf("@INGAMETEST passed=%u failed=%u\n", passed, failed);
}
#endif
