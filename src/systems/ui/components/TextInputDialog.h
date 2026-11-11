#pragma once
#include <functional>
#include <string>
#include "BrandHeader.h"
#include "SceneLayout.h"
#include "Container.h"
#include "Button.h"

// One field at a time leaves room for a full keyboard on the 320x240 display.
class TextInputDialog
{
public:
    void create(lv_obj_t *parent, std::function<void(const std::string &)> accepted)
    {
        callback = std::move(accepted);
        root = Container::create(parent, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_LEFT);
        lv_obj_add_flag(root, LV_OBJ_FLAG_FLOATING); UITheme::screen(root);
        lv_obj_set_style_bg_opa(root, LV_OPA_TRANSP, 0);
        auto layout = SceneLayout::create(root, 44, 108, false);
        // Opaque rows cover the underlying form while preserving the global
        // status indicators above the dialog (including LVGL snapshots).
        for (auto *row : {layout.top, layout.body, layout.footer})
        {
            UITheme::screen(row);
            lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        }
        auto *header = BrandHeader::create(layout.top, 100, "SSID"); title = lv_obj_get_child(header, 1);
        lv_obj_set_style_pad_ver(layout.body, 0, 0);
        Container::setFlex(layout.body); lv_obj_set_style_pad_row(layout.body, 0, 0);
        auto *inputRow = Container::create(layout.body, LV_PCT(100), 44);
        Container::setFlex(inputRow, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(inputRow, 6, 0);
        input = lv_textarea_create(inputRow); lv_obj_set_size(input, 0, 44); lv_obj_set_flex_grow(input, 1);
        lv_textarea_set_one_line(input, true);
        visibility = Button::create(inputRow, nullptr, onVisibility, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_EYE_OPEN);
        UITheme::secondary(visibility);
        auto *actions = Container::create(layout.body, LV_PCT(100), 44);
        Container::setFlex(actions, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(actions, 8, 0);
        auto *cancel = Button::create(actions, tr(Text::Cancel), onCancel, this, 140, 44);
        UITheme::secondary(cancel);
        Button::create(actions, tr(Text::Confirm), onReady, this, 140, 44);
        keyboard = lv_keyboard_create(layout.footer); lv_obj_set_size(keyboard, LV_PCT(100), LV_PCT(100));
        lv_keyboard_set_textarea(keyboard, input);
        lv_obj_add_event_cb(keyboard, onReady, LV_EVENT_READY, this);
        lv_obj_add_event_cb(keyboard, onCancel, LV_EVENT_CANCEL, this);
        hide();
    }
    void show(const char *caption, const std::string &value, bool password)
    {
        Label::setText(title, caption);
        lv_textarea_set_max_length(input, password ? 64 : 32);
        lv_textarea_set_password_mode(input, password);
        lv_textarea_set_password_show_time(input, 0);
        lv_textarea_set_text(input, value.c_str());
        lv_obj_set_flag(visibility, LV_OBJ_FLAG_HIDDEN, !password);
        Label::setText(lv_obj_get_child(visibility,0), LV_SYMBOL_EYE_OPEN);
        lv_obj_remove_state(visibility, LV_STATE_CHECKED);
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN); lv_obj_move_foreground(root);
    }
    void hide()
    {
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
        lv_textarea_set_password_mode(input, true);
        lv_textarea_set_text(input, "");
    }
    void clear() { root = title = input = keyboard = visibility = nullptr; callback = nullptr; }
private:
    static void onVisibility(lv_event_t *e)
    {
        auto *self = static_cast<TextInputDialog *>(lv_event_get_user_data(e));
        const bool visible = lv_textarea_get_password_mode(self->input);
        lv_textarea_set_password_mode(self->input, !visible);
        lv_obj_set_state(self->visibility, LV_STATE_CHECKED, visible);
        Label::setText(lv_obj_get_child(self->visibility,0), visible ? LV_SYMBOL_EYE_CLOSE : LV_SYMBOL_EYE_OPEN);
    }
    static void onReady(lv_event_t *e)
    {
        auto *self = static_cast<TextInputDialog *>(lv_event_get_user_data(e));
        if (self->callback) self->callback(lv_textarea_get_text(self->input));
        self->hide();
    }
    static void onCancel(lv_event_t *e) { static_cast<TextInputDialog *>(lv_event_get_user_data(e))->hide(); }
    lv_obj_t *root = nullptr, *title = nullptr, *input = nullptr, *keyboard = nullptr, *visibility = nullptr;
    std::function<void(const std::string &)> callback;
};
