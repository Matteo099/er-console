#pragma once

#include <lvgl.h>

class BackButton {
public:
    static lv_obj_t* create(
        lv_obj_t* parent,
        lv_event_cb_t callback,
        void* userData);
};
