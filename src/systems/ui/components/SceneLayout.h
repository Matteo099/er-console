#pragma once

#include <lvgl.h>

class SceneLayout
{
public:
    struct Parts
    {
        lv_obj_t* top = nullptr;
        lv_obj_t* body = nullptr;
        lv_obj_t* footer = nullptr;
    };

    static Parts create(
        lv_obj_t* screen,
        lv_coord_t topHeight = 56,
        lv_coord_t footerHeight = 56,
        bool bodyScrollable = true)
    {
        Parts parts;
        if (!screen)
            return parts;

        lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_all(screen, 0, 0);
        lv_obj_set_style_pad_row(screen, 0, 0);
        lv_obj_set_scroll_dir(screen, LV_DIR_NONE);

        parts.top = createRow(screen, topHeight, false);
        parts.body = createRow(screen, 0, bodyScrollable);
        lv_obj_set_flex_grow(parts.body, 1);
        parts.footer = createRow(screen, footerHeight, false);

        return parts;
    }

private:
    static lv_obj_t* createRow(lv_obj_t* parent, lv_coord_t height, bool scrollable)
    {
        lv_obj_t* row = lv_obj_create(parent);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_height(row, height);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_outline_width(row, 0, 0);
        lv_obj_set_style_shadow_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);

        if (scrollable)
        {
            lv_obj_add_flag(row, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_scroll_dir(row, LV_DIR_VER);
            lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_AUTO);
        }

        return row;
    }
};