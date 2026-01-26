#include "LVGLTouch.h"

#include "drivers/touch/TouchController.h"

static TouchController* touch = nullptr;

void LVGLTouch::vgl_touch_read(lv_indev_drv_t* drv, lv_indev_data_t* data) {

    TouchPoint p = touch->read();

    data->state = p.pressed ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    data->point.x = p.x;
    data->point.y = p.y;
}

void LVGLTouch::lvgl_register_touch(TouchController* controller) {

    touch = controller;

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_read;

    lv_indev_drv_register(&indev_drv);
}
