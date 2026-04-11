#pragma once

#include <lvgl.h>
#include "ImageView.h"
#include "config/Assets.h"

class BrandHeader
{
public:
    static lv_obj_t* create(lv_obj_t* parent)
    {
        return ImageView::create(parent, IMG_BRAND_FILE, LV_ALIGN_TOP_MID, 0, 12);
    }
};
