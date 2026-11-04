#pragma once

#include <lvgl.h>
#include "drivers/display/DisplayDriver.h"
#include "drivers/display/TouchController.h"

class LVGLContext
{
public:
    void init(DisplayDriver* display, TouchController* touch);
    void update();

private:
    static void flush_cb(
        lv_display_t* disp,
        const lv_area_t* area,
        uint8_t* px_map
    );

    static void touch_read_cb(
        lv_indev_t* indev,
        lv_indev_data_t* data
    );
    static void flush_wait_cb(lv_display_t *disp);

    static DisplayDriver* s_display;
    static TouchController* s_touch;

    lv_display_t* display = nullptr;
    lv_indev_t* touchIndev = nullptr;
};
