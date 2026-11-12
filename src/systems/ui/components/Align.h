#pragma once

#include <lvgl.h>

class Align {
  public:
    static void setPosition(lv_obj_t* obj, lv_align_t align, lv_coord_t x = 0, lv_coord_t y = 0) {
        if (obj)
            lv_obj_align(obj, align, x, y);
    }

    static void alignTo(lv_obj_t* obj, lv_obj_t* reference, lv_align_t align, lv_coord_t x = 0,
                        lv_coord_t y = 0) {
        if (obj && reference)
            lv_obj_align_to(obj, reference, align, x, y);
    }
};
