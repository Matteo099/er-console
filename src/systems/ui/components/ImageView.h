#pragma once

#include <lvgl.h>
#include <string>

namespace {
inline std::string normalizeImageViewSrc(const char* src) {
    if (!src || !*src)
        return {};

    if ((src[0] >= 'A' && src[0] <= 'Z') || (src[0] >= 'a' && src[0] <= 'z')) {
        if (src[1] == ':')
            return src;
    }

    return std::string("S:") + src;
}
}  // namespace

class ImageView {
  public:
    static lv_obj_t* create(lv_obj_t* parent, const char* src, lv_align_t align = LV_ALIGN_CENTER,
                            lv_coord_t x = 0, lv_coord_t y = 0,
                            uint16_t zoom = 256)  // 256 = 100%
    {
        if (!parent || !src || !*src)
            return nullptr;

        lv_obj_t* image = lv_img_create(parent);
        if (!image)
            return nullptr;

        const std::string resolvedSrc = normalizeImageViewSrc(src);

        lv_img_set_src(image, resolvedSrc.c_str());
        lv_img_set_zoom(image, zoom);

        lv_obj_align(image, align, x, y);

        // Hardening for UI stability
        lv_obj_clear_flag(image, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_clear_flag(image, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(image, LV_OBJ_FLAG_GESTURE_BUBBLE);
        lv_obj_clear_flag(image, LV_OBJ_FLAG_SCROLL_CHAIN);

        return image;
    }
};