#pragma once

#include <lvgl.h>
#include <cstring>
#include "systems/ui/UITheme.h"
#include "systems/ui/localization/LocalizedText.h"

class Label {
  public:
    static lv_obj_t* create(lv_obj_t* parent, const char* text, const lv_font_t* font = nullptr,
                            lv_text_align_t align = LV_TEXT_ALIGN_LEFT,
                            lv_coord_t width = LV_SIZE_CONTENT, lv_coord_t x = LV_COORD_MAX,
                            lv_coord_t y = LV_COORD_MAX) {
        lv_obj_t* label = lv_label_create(parent);
        lv_label_set_text(label, text);
        LocalizedText::bind(label, text);

        if (font)
            lv_obj_set_style_text_font(label, UITheme::font(font), 0);

        lv_obj_set_width(label, width);

        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

        lv_obj_set_style_text_align(label, align, 0);

        // ONLY apply position if user explicitly set it
        if (x != LV_COORD_MAX && y != LV_COORD_MAX) {
            lv_obj_set_pos(label, x, y);
        }

        return label;
    }

    static void setFont(lv_obj_t* label, const lv_font_t* font) {
        if (label && font)
            lv_obj_set_style_text_font(label, UITheme::font(font), 0);
    }

    static void setTextColor(lv_obj_t* label, lv_color_t color) {
        if (label)
            lv_obj_set_style_text_color(label, color, 0);
    }

    static void setText(lv_obj_t* label, const char* text) {
        if (label && text && std::strcmp(lv_label_get_text(label), text)) {
            LocalizedText::bind(label, text);
            lv_label_set_text(label, text);
        }
    }
};
