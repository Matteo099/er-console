#pragma once
#include <lvgl.h>
#include "systems/ui/UITheme.h"
#include "config/Branding.h"
#include "LimanulabLogo.h"

// Vector/text chrome renders immediately without reading SD assets.
class BrandHeader
{
public:
    static lv_obj_t *create(lv_obj_t *parent, uint8_t = 100, const char *title = CONSOLE_TITLE)
    {
        auto *row = lv_obj_create(parent);
        lv_obj_remove_style_all(row);
        lv_obj_set_size(row, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_pad_hor(row, 12, 0);
        lv_obj_set_style_pad_gap(row, 9, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        LimanulabLogo::create(row);
        auto *label = lv_label_create(row);
        lv_label_set_text(label, title);
        lv_obj_set_style_text_font(label, UITheme::font(&lv_font_montserrat_16), 0);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_flex_grow(label, 1);
        return row;
    }
};
