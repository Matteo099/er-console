#include "BrandHeader.h"

lv_obj_t* BrandHeader::create(lv_obj_t* parent)
{
    lv_obj_t* img = lv_img_create(parent);
    lv_img_set_src(img, "S:/brand.png");
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 12);
    return img;
}
