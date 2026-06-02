#pragma once

#include <lvgl.h>
#include "ImageView.h"
#include "config/Assets.h"

class BrandHeader
{
public:
    static lv_obj_t* create(lv_obj_t* parent, uint8_t scalePercent = 100)
    {
        if (scalePercent == 0)
            scalePercent = 1;
        else if (scalePercent > 100)
            scalePercent = 100;

        const uint16_t zoom = static_cast<uint16_t>((static_cast<uint32_t>(scalePercent) * 256U + 50U) / 100U);

        lv_obj_t* image = ImageView::create(parent, IMG_BRAND_FILE, LV_ALIGN_TOP_MID, 0, 12, zoom);

        lv_coord_t availableWidth = lv_obj_get_width(parent);
        if (availableWidth <= 0)
        {
            lv_display_t* display = lv_obj_get_display(parent);
            if (!display)
                display = lv_display_get_default();

            if (display)
                availableWidth = lv_display_get_horizontal_resolution(display);
        }

        if (availableWidth > 24)
            availableWidth -= 24;

        const lv_coord_t parentWidth = availableWidth;
        const lv_coord_t imageWidth = lv_obj_get_width(image);
        if (parentWidth > 0 && imageWidth > parentWidth)
        {
            const uint16_t fitZoom = static_cast<uint16_t>((static_cast<int32_t>(parentWidth) * 256U) / imageWidth);
            lv_img_set_zoom(image, fitZoom > 0 ? fitZoom : 1);
        }

        return image;
    }
};
