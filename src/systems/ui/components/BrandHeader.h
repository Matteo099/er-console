#pragma once

#include <lvgl.h>
#include "ImageView.h"

class BrandHeader
{
public:
    static lv_obj_t* create(lv_obj_t* parent)
    {
        return ImageView::create(parent, "S:/brand.png", LV_ALIGN_TOP_MID, 0, 12);
    }
};
