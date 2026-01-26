#pragma once

#include <lvgl.h>

class LVGLTouch
{
public:
    void lvgl_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

    void lvgl_register_touch(TouchController *controller);
};