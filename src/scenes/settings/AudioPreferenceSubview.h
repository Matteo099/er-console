#pragma once
#include <functional>
#include "systems/audio/AudioPreferences.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/localization/I18n.h"

// Reused by music and effects; only publishes values, never calls audio services.
class AudioPreferenceSubview
{
public:
    void create(lv_obj_t *parent, Text title, std::function<void(AudioChannelPreferences, bool)> changed)
    {
        titleKey = title; callback = std::move(changed);
        auto *card = Container::create(parent, LV_PCT(100), LV_SIZE_CONTENT);
        UITheme::card(card); Container::setFlex(card);
        lv_obj_set_style_pad_all(card, 8, 0); lv_obj_set_style_pad_row(card, 2, 0);
        auto *row = Container::create(card, LV_PCT(100), 28);
        Container::setFlex(row, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER);
        titleLabel = Label::create(row, tr(title), UITheme::heading(18));
        toggle = lv_switch_create(row); lv_obj_set_size(toggle, 44, 25); lv_obj_set_ext_click_area(toggle, 9);
        lv_obj_add_event_cb(toggle, onChanged, LV_EVENT_VALUE_CHANGED, this);
        auto *volumeRow = Container::create(card, LV_PCT(100), 44);
        Container::setFlex(volumeRow, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(volumeRow, 12, 0);
        slider = lv_slider_create(volumeRow); lv_obj_set_height(slider, 12); lv_obj_set_flex_grow(slider, 1);
        lv_obj_set_ext_click_area(slider, 16); lv_slider_set_range(slider, 0, 100);
        lv_obj_add_event_cb(slider, onChanged, LV_EVENT_VALUE_CHANGED, this);
        volumeLabel = Label::create(volumeRow, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_RIGHT, 44);
        set({true,70});
    }
    void set(AudioChannelPreferences value)
    {
        lv_obj_set_state(toggle, LV_STATE_CHECKED, value.enabled);
        lv_slider_set_value(slider, value.volume, LV_ANIM_OFF);
        updateLabels();
    }
    void translate() { Label::setText(titleLabel, tr(titleKey)); updateLabels(); }
    AudioChannelPreferences value() const { return {lv_obj_has_state(toggle, LV_STATE_CHECKED), uint8_t(lv_slider_get_value(slider))}; }
    void clear() { toggle = slider = volumeLabel = titleLabel = nullptr; callback = nullptr; }
private:
    void updateLabels()
    {
        char text[16]; lv_snprintf(text, sizeof(text), "%u%%", unsigned(lv_slider_get_value(slider)));
        Label::setText(volumeLabel, text);
    }
    static void onChanged(lv_event_t *e)
    {
        auto *self = static_cast<AudioPreferenceSubview *>(lv_event_get_user_data(e));
        self->updateLabels(); if (self->callback) self->callback(self->value(), lv_event_get_target(e) == self->slider);
    }
    Text titleKey = Text::None;
    lv_obj_t *toggle = nullptr, *slider = nullptr, *volumeLabel = nullptr, *titleLabel = nullptr;
    std::function<void(AudioChannelPreferences, bool)> callback;
};
