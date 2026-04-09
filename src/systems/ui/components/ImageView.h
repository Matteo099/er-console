#pragma once

#include <lvgl.h>

class ImageView
{
public:
    static lv_obj_t* create(
        lv_obj_t* parent,
        const char* src,
        lv_align_t align = LV_ALIGN_CENTER,
        lv_coord_t x = 0,
        lv_coord_t y = 0)
    {
        lv_obj_t* image = lv_img_create(parent);
        lv_img_set_src(image, src);
        lv_obj_align(image, align, x, y);
        return image;
    }
};
