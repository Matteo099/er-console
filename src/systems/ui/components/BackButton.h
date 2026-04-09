#pragma once

#include <lvgl.h>
#include "Button.h"

class BackButton
{
public:
    static lv_obj_t* create(
        lv_obj_t* parent,
        lv_event_cb_t callback,
        void* userData)
    {
        return Button::create(parent, "Back", callback, userData, 90, 36, LV_ALIGN_BOTTOM_MID, 0, -12);
    }
};
