#pragma once
#include <functional>
#include "Container.h"
#include "Label.h"
#include "Button.h"

class ConfirmationDialog
{
public:
    void create(lv_obj_t *parent, const char *question, std::function<void(bool)> handler)
    {
        callback = std::move(handler);
        root = Container::create(parent, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_LEFT);
        lv_obj_add_flag(root, LV_OBJ_FLAG_FLOATING);
        lv_obj_set_style_bg_color(root, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(root, LV_OPA_40, 0);
        auto *card = Container::create(root, LV_PCT(92), LV_SIZE_CONTENT, LV_ALIGN_CENTER, 0, 16);
        UITheme::card(card);
        Container::setFlex(card);
        lv_obj_set_style_pad_row(card, 12, 0);
        Label::create(card, "Abbandonare il gioco?", &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER, LV_PCT(100));
        auto *text = Label::create(card, question, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER, LV_PCT(100));
        UITheme::muted(text);
        auto *row = Container::create(card, LV_PCT(100), 44);
        Container::setFlex(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(row, 8, 0);
        auto *stay = Button::create(row, "Continua", onStay, this, 0, 44);
        auto *leave = Button::create(row, "Abbandona", onLeave, this, 0, 44);
        lv_obj_set_flex_grow(stay, 1);
        lv_obj_set_flex_grow(leave, 1);
        UITheme::secondary(leave);
        UITheme::danger(lv_obj_get_child(leave, 0));
        hide();
    }
    void show() { lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN); lv_obj_move_foreground(root); }
    void hide() { if (root) lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN); }
    void destroy() { root = nullptr; callback = nullptr; }
private:
    static void onStay(lv_event_t *e) { auto *self = static_cast<ConfirmationDialog *>(lv_event_get_user_data(e)); if (self->callback) self->callback(false); }
    static void onLeave(lv_event_t *e) { auto *self = static_cast<ConfirmationDialog *>(lv_event_get_user_data(e)); if (self->callback) self->callback(true); }
    lv_obj_t *root = nullptr;
    std::function<void(bool)> callback;
};
