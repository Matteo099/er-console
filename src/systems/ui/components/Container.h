#pragma once

#include <lvgl.h>

class Container
{
public:
    static lv_obj_t* create(
        lv_obj_t* parent,
        lv_coord_t width,
        lv_coord_t height,
        lv_align_t align = LV_ALIGN_CENTER,
        lv_coord_t x = 0,
        lv_coord_t y = 0)
    {
        lv_obj_t* container = lv_obj_create(parent);
        lv_obj_set_size(container, width, height);
        lv_obj_align(container, align, x, y);
        lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
        lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
        return container;
    }

    static void setFlex(
        lv_obj_t* object,
        lv_flex_flow_t flow = LV_FLEX_FLOW_COLUMN,
        lv_flex_align_t mainAxis = LV_FLEX_ALIGN_CENTER,
        lv_flex_align_t crossAxis = LV_FLEX_ALIGN_CENTER)
    {
        if (!object)
            return;

        lv_obj_set_flex_flow(object, flow);
        lv_obj_set_flex_align(object, mainAxis, crossAxis, crossAxis);
    }
};
