#pragma once
#include <lvgl.h>
#include "systems/ui/UITheme.h"

// Vector/text chrome renders immediately without reading SD assets.
class BrandHeader
{
public:
    static lv_obj_t *create(lv_obj_t *parent, uint8_t = 100, const char *title = "EscapeOS")
    {
        auto *row = lv_obj_create(parent);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_pad_hor(row, 12, 0);
        lv_obj_set_style_pad_gap(row, 9, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        auto *mark = lv_obj_create(row);
        lv_obj_remove_style_all(mark);
        UITheme::soft(mark);
        lv_obj_set_size(mark, 30, 28);
        lv_obj_set_style_radius(mark, 9, 0);
        auto *initials = lv_label_create(mark);
        lv_label_set_text(initials, "ER");
        lv_obj_set_style_text_font(initials, &lv_font_montserrat_12, 0);
        UITheme::accent(initials);
        lv_obj_center(initials);
        auto *label = lv_label_create(row);
        lv_label_set_text(label, title);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_flex_grow(label, 1);
        return row;
    }
};
