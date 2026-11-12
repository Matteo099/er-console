#pragma once
#include "scenes/settings/AudioPreferenceSubview.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/SceneLayout.h"

// An overlay owned by GameView, not a new Scene. The underlying subview and
// its widgets remain alive, including scroll positions and object selection.
class InGameSettingsSubview {
  public:
    void create(lv_obj_t* parent, std::function<void(const UIEvent&)> emit) {
        callback = std::move(emit);
        root = Container::create(parent, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_LEFT);
        lv_obj_add_flag(root, LV_OBJ_FLAG_FLOATING);
        lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);
        UITheme::screen(root);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
        auto layout = SceneLayout::create(root, 44, 48, true);
        auto* header = BrandHeader::create(layout.top, 100, tr(Text::Settings));
        title = lv_obj_get_child(header, 1);
        Container::setFlex(layout.body, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(layout.body, 6, 0);
        hint = Label::create(layout.body, tr(Text::GameSettingsRunning), &lv_font_montserrat_12,
                             LV_TEXT_ALIGN_LEFT, LV_PCT(100));
        UITheme::muted(hint);
        time = Label::create(layout.footer, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, 90);
        lv_obj_align(time, LV_ALIGN_LEFT_MID, 10, 0);
        music.create(layout.body, Text::Music, [this](AudioChannelPreferences value, bool preview) {
            if (callback)
                callback(AudioPreferenceChangedEvent(AudioChannel::Music, value, preview));
        });
        effects.create(layout.body, Text::Effects,
                       [this](AudioChannelPreferences value, bool preview) {
                           if (callback)
                               callback(AudioPreferenceChangedEvent(AudioChannel::Effects, value,
                                                                    preview));
                       });
        status = Label::create(layout.body, tr(Text::Saved), &lv_font_montserrat_12);
        UITheme::muted(status);
        back = Button::create(layout.footer, tr(Text::BackToGame), onBack, this, 200, 44,
                              LV_ALIGN_RIGHT_MID, -8, 0, LV_SYMBOL_LEFT);
        UITheme::secondary(back);
        hide();
    }

    void show(const AudioPreferences& preferences, Text context) {
        set(preferences);
        contextText = context;
        translate();
        setStatus(Text::Saved);
        lv_obj_scroll_to_y(lv_obj_get_child(root, 1), 0, LV_ANIM_OFF);
        lv_obj_remove_flag(root, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(root);
    }

    void set(const AudioPreferences& value) {
        music.set(value.music);
        effects.set(value.effects);
    }

    void setTime(uint32_t seconds) {
        char text[16];
        lv_snprintf(text, sizeof(text), "%02lu:%02lu:%02lu",
                    static_cast<unsigned long>(seconds / 3600),
                    static_cast<unsigned long>((seconds % 3600) / 60),
                    static_cast<unsigned long>(seconds % 60));
        Label::setText(time, text);
    }

    void setStatus(Text value) {
        statusText = value;
        Label::setText(status, tr(value));
    }

    void translate() {
        Label::setText(title, tr(Text::Settings));
        Label::setText(hint, tr(contextText));
        Label::setText(lv_obj_get_child(back, 1), tr(Text::BackToGame));
        music.translate();
        effects.translate();
        setStatus(statusText);
    }

    bool visible() const {
        return root && !lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN);
    }

    void hide() {
        if (root)
            lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
    }

    void clear() {
        music.clear();
        effects.clear();
        root = title = hint = time = status = back = nullptr;
        callback = nullptr;
    }

  private:
    static void onBack(lv_event_t* event) {
        auto* self = static_cast<InGameSettingsSubview*>(lv_event_get_user_data(event));
        if (self->callback)
            self->callback(ButtonClickedEvent("close_game_settings"));
    }

    lv_obj_t *root = nullptr, *title = nullptr, *hint = nullptr, *time = nullptr, *status = nullptr,
             *back = nullptr;
    Text contextText = Text::GameSettingsRunning, statusText = Text::Saved;
    AudioPreferenceSubview music, effects;
    std::function<void(const UIEvent&)> callback;
};
