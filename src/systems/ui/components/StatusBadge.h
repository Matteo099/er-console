#pragma once
#include "Container.h"
#include "Label.h"

class StatusBadge
{
public:
    struct Parts { lv_obj_t *root = nullptr, *icon = nullptr, *caption = nullptr, *cross = nullptr; };

    static Parts create(lv_obj_t *parent, const char *symbol, const char *text)
    {
        Parts parts;
        parts.root = Container::create(parent, LV_SIZE_CONTENT, 20);
        lv_obj_clear_flag(parts.root, LV_OBJ_FLAG_CLICKABLE);
        Container::setFlex(parts.root, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(parts.root, 3, 0);
        auto *glyph = Container::create(parts.root, 18, 18);
        lv_obj_clear_flag(glyph, LV_OBJ_FLAG_CLICKABLE);
        parts.icon = Label::create(glyph, symbol, &lv_font_montserrat_12);
        lv_obj_center(parts.icon);
        parts.caption = Label::create(parts.root, text, &lv_font_montserrat_12);
        parts.cross = lv_obj_create(glyph);
        lv_obj_remove_style_all(parts.cross);
        lv_obj_clear_flag(parts.cross, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_size(parts.cross, 9, 9);
        lv_obj_align(parts.cross, LV_ALIGN_TOP_RIGHT, 0, 0);
        lv_obj_set_style_radius(parts.cross, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_opa(parts.cross, LV_OPA_COVER, 0);
        auto *cross = Label::create(parts.cross, LV_SYMBOL_CLOSE, &lv_font_montserrat_8);
        lv_obj_set_style_text_color(cross, lv_color_white(), 0);
        lv_obj_center(cross);
        set(parts, UIStatusTone::Good, false, text);
        return parts;
    }

    static void set(const Parts &parts, UIStatusTone tone, bool unavailable, const char *text)
    {
        if (!parts.root) return;
        const auto color = UITheme::statusColor(tone);
        lv_obj_set_style_text_color(parts.icon, color, 0);
        lv_obj_set_style_text_color(parts.caption, color, 0);
        lv_obj_set_style_bg_color(parts.cross, color, 0);
        lv_obj_set_flag(parts.cross, LV_OBJ_FLAG_HIDDEN, !unavailable);
        Label::setText(parts.caption, text);
    }
};
