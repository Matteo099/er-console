#pragma once

#include <lvgl.h>

class Screen
{
public:
    static lv_obj_t* create()
    {
        lv_obj_t* screen = lv_obj_create(nullptr);
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(screen, 0, 0);
        lv_obj_set_style_outline_width(screen, 0, 0);
        lv_obj_set_style_shadow_width(screen, 0, 0);
        lv_obj_set_style_pad_all(screen, 0, 0);
        return screen;
    }
};
