#pragma once

#include <lvgl.h>

class Container
{
public:
    static lv_obj_t* create(
        lv_obj_t* parent,
        lv_coord_t width = LV_PCT(100),
        lv_coord_t height = LV_SIZE_CONTENT,
        lv_align_t align = LV_ALIGN_CENTER,
        lv_coord_t x = 0,
        lv_coord_t y = 0)
    {
        lv_obj_t* container = lv_obj_create(parent);

        lv_obj_set_size(container, width, height);
        lv_obj_align(container, align, x, y);

        // visuals
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_outline_width(container, 0, 0);
        lv_obj_set_style_shadow_width(container, 0, 0);

        // IMPORTANT: allow layout overflow handling
        lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scrollbar_mode(container, LV_SCROLLBAR_MODE_OFF);

        // default padding (safe UI default)
        lv_obj_set_style_pad_all(container, 0, 0);

        return container;
    }

    static void setFlex(
        lv_obj_t* object,
        lv_flex_flow_t flow = LV_FLEX_FLOW_COLUMN,
        lv_flex_align_t mainAxis = LV_FLEX_ALIGN_START,
        lv_flex_align_t crossAxis = LV_FLEX_ALIGN_CENTER)
    {
        if (!object)
            return;

        lv_obj_set_flex_flow(object, flow);

        // FIXED: correct LVGL API usage
        lv_obj_set_flex_align(
            object,
            mainAxis,
            crossAxis,
            crossAxis
        );
    }

    static void setScrollable(lv_obj_t* object)
    {
        if (!object)
            return;

        lv_obj_add_flag(object, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scroll_dir(object, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(object, LV_SCROLLBAR_MODE_AUTO);
    }
};