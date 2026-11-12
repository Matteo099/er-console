#pragma once
#include <lvgl.h>
#include "systems/ui/UITheme.h"
#include "systems/ui/UIFeedback.h"
#include "systems/audio/AudioEvent.h"
#include "config/Assets.h"
#include "systems/ui/localization/LocalizedText.h"
#include "ButtonClickGuard.h"

class Button {
  public:
    static void playClick(lv_event_t*) {
        if (UIFeedback::soundEnabled())
            sendAudioEvent(AUDIO_CLICK1_FILE, AudioEventType::PLAY_WAV);
    }

    static lv_obj_t* create(lv_obj_t* parent, const char* text, lv_event_cb_t callback, void* data,
                            lv_coord_t width = 200, lv_coord_t height = 44,
                            lv_align_t align = LV_ALIGN_CENTER, lv_coord_t x = 0, lv_coord_t y = 0,
                            const char* icon = nullptr) {
        auto* button = lv_button_create(parent);
        ButtonClickGuard::attach(button);
        lv_obj_set_size(button, width, height < 44 ? 44 : height);
        lv_obj_align(button, align, x, y);
        lv_obj_set_flex_flow(button, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_gap(button, 8, 0);
        lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);
        if (callback)
            lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, data);
        lv_obj_add_event_cb(button, playClick, LV_EVENT_CLICKED, nullptr);
        if (icon) {
            auto* label = lv_label_create(button);
            lv_label_set_text(label, icon);
            lv_obj_set_style_text_font(label, UITheme::font(&lv_font_montserrat_18), 0);
        }
        if (text) {
            auto* label = lv_label_create(button);
            lv_label_set_text(label, text);
            LocalizedText::bind(label, text);
            lv_obj_set_style_text_font(label, UITheme::font(&lv_font_montserrat_14), 0);
            lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
            lv_obj_set_flex_grow(label, 1);
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        }
        return button;
    }
};
