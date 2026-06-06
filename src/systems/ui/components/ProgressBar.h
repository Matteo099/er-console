#pragma once

#include <lvgl.h>

class ProgressBar
{
public:
    static lv_obj_t *create(
        lv_obj_t *parent,
        lv_coord_t width = 220,
        lv_coord_t height = 14,
        lv_color_t bgColor = lv_color_hex(0x2A2A2A),
        lv_color_t indicatorColor = lv_color_hex(0x00FF88),
        lv_align_t align = LV_ALIGN_CENTER,
        lv_coord_t x = 0,
        lv_coord_t y = 0,
        uint16_t animTime = 300) // 👈 add this
    {
        lv_obj_t *bar = lv_bar_create(parent);
        lv_obj_set_size(bar, width, height);
        lv_obj_align(bar, align, x, y);

        lv_bar_set_range(bar, 0, 100);
        lv_bar_set_value(bar, 0, LV_ANIM_OFF);

        lv_obj_set_style_anim_time(bar, animTime, 0);

        lv_obj_set_style_bg_color(bar, lv_theme_get_color_secondary(bar), LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, lv_theme_get_color_primary(bar), LV_PART_INDICATOR);
        // lv_obj_set_style_bg_color(bar, bgColor, LV_PART_MAIN);
        // lv_obj_set_style_bg_color(bar, indicatorColor, LV_PART_INDICATOR);
        lv_obj_set_style_radius(bar, 7, LV_PART_MAIN | LV_PART_INDICATOR);

        return bar;
    }

    static void setValue(lv_obj_t *bar, uint32_t value, lv_anim_enable_t anim = LV_ANIM_ON)
    {
        if (bar)
            lv_bar_set_value(bar, value, anim);
    }

    static uint32_t getValue(lv_obj_t *bar)
    {
        if (bar)
            return lv_bar_get_value(bar);
        return 0;
    }

    static void setColors(
        lv_obj_t *bar,
        lv_color_t bgColor,
        lv_color_t indicatorColor)
    {
        if (!bar)
            return;

        lv_obj_set_style_bg_color(bar, bgColor, LV_PART_MAIN);
        lv_obj_set_style_bg_color(bar, indicatorColor, LV_PART_INDICATOR);
    }
};
