#pragma once

#include <lvgl.h>

class Button
{
public:
    static lv_obj_t* create(
        lv_obj_t* parent,
        const char* label,
        lv_event_cb_t callback,
        void* userData,
        lv_coord_t width = 200,
        lv_coord_t height = 42,
        lv_align_t align = LV_ALIGN_CENTER,
        lv_coord_t x = 0,
        lv_coord_t y = 0)
    {
        lv_obj_t* btn = lv_btn_create(parent);
        lv_obj_set_size(btn, width, height);
        lv_obj_align(btn, align, x, y);
        lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, userData);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, label);
        lv_obj_center(lbl);

        return btn;
    }
};
