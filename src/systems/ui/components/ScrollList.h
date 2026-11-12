#pragma once

#include <lvgl.h>

class ScrollList {
  public:
    static lv_obj_t* create(lv_obj_t* parent, lv_coord_t width, lv_coord_t height,
                            lv_align_t align = LV_ALIGN_CENTER, lv_coord_t x = 0,
                            lv_coord_t y = 0) {
        lv_obj_t* list = lv_obj_create(parent);
        lv_obj_set_size(list, width, height);
        lv_obj_align(list, align, x, y);
        lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
        lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
        lv_obj_set_scroll_dir(list, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);
        return list;
    }
};
