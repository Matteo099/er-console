#include "LVGLContext.h"

static const uint32_t LVGL_TICK_PERIOD_MS = 5;

DisplayDriver* LVGLContext::s_display = nullptr;
TouchController* LVGLContext::s_touch = nullptr;

void LVGLContext::init(DisplayDriver* displayDrv, TouchController* touchDrv)
{
    s_display = displayDrv;
    s_touch   = touchDrv;

    lv_init();

    // --- Display ---
    display = lv_display_create(
        displayDrv->width(),
        displayDrv->height()
    );

    static lv_color_t buf1[320 * 40];
    static lv_color_t buf2[320 * 40];

    lv_display_set_buffers(
        display,
        buf1,
        buf2,
        sizeof(buf1),
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    lv_display_set_flush_cb(display, flush_cb);

    // --- Touch ---
    touchIndev = lv_indev_create();
    lv_indev_set_type(touchIndev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touchIndev, touch_read_cb);
}

void LVGLContext::update()
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
    lv_timer_handler();
}

void LVGLContext::flush_cb(
    lv_display_t* disp,
    const lv_area_t* area,
    uint8_t* px_map
)
{
    lv_display_flush_ready(disp);

    s_display->flush(
        area->x1,
        area->y1,
        area->x2,
        area->y2,
        reinterpret_cast<uint16_t*>(px_map)
    );
}

void LVGLContext::touch_read_cb(
    lv_indev_t* indev,
    lv_indev_data_t* data
)
{
    TouchPoint p = s_touch->read();

    data->state = p.pressed
        ? LV_INDEV_STATE_PRESSED
        : LV_INDEV_STATE_RELEASED;

    data->point.x = p.x;
    data->point.y = p.y;
}
