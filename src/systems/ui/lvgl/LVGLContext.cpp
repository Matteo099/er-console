#include "LVGLContext.h"
#include "log/Logger.h"

static const uint32_t LVGL_TICK_PERIOD_MS = 5;

DisplayDriver *LVGLContext::s_display = nullptr;
TouchController *LVGLContext::s_touch = nullptr;

void LVGLContext::init(DisplayDriver *displayDrv, TouchController *touchDrv)
{
    s_display = displayDrv;
    s_touch = touchDrv;

    Log::info(Log::DISPLAY_TAG, "Initializing LVGL Context");

    // --- LVGL core init ---
    lv_init();
    Log::debug(Log::DISPLAY_TAG, "LVGL core initialized");

    // --- Display setup ---
    int disp_width = displayDrv->width();
    int disp_height = displayDrv->height();
    Log::debug(Log::DISPLAY_TAG, "DisplayDriver reports width=%d, height=%d", disp_width, disp_height);

    if (disp_width <= 0 || disp_height <= 0)
    {
        Log::error(Log::DISPLAY_TAG, "Invalid display size! LVGL will not work correctly.");
        disp_width = 240;  // fallback
        disp_height = 320; // fallback
    }

    display = lv_display_create(disp_width, disp_height);
    Log::debug(Log::DISPLAY_TAG, "LVGL display object created");

    // --- Buffers ---
    lv_color_t *buf1 = new lv_color_t[disp_width * 40];
    lv_color_t *buf2 = new lv_color_t[disp_height * 40];

    Log::debug(Log::DISPLAY_TAG, "Setting LVGL display buffers: buf1=%p buf2=%p size=%d bytes",
               buf1, buf2, sizeof(buf1));

    lv_display_set_buffers(
        display,
        buf1,
        buf2,
        disp_width * 40 * sizeof(lv_color_t),
        LV_DISPLAY_RENDER_MODE_PARTIAL);

    Log::debug(Log::DISPLAY_TAG, "LVGL display buffers configured");

    // --- Flush callback ---
    lv_display_set_flush_cb(display, flush_cb);
    Log::debug(Log::DISPLAY_TAG, "LVGL flush callback set");

    // --- Touch input ---
    touchIndev = lv_indev_create();
    lv_indev_set_type(touchIndev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touchIndev, touch_read_cb);
    Log::debug(Log::DISPLAY_TAG, "LVGL touch input device created");

    if (!s_touch)
    {
        Log::warn(Log::DISPLAY_TAG, "TouchController is NULL; touch input will not work");
    }

    Log::info(Log::DISPLAY_TAG, "LVGL Context initialization complete");
}

void LVGLContext::update()
{
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
    lv_timer_handler();
}

void LVGLContext::flush_cb(
    lv_display_t *disp,
    const lv_area_t *area,
    uint8_t *px_map)
{
    // Draw first
    s_display->flush(
        area->x1,
        area->y1,
        area->x2,
        area->y2,
        reinterpret_cast<uint16_t *>(px_map));

    // Then tell LVGL we are done
    lv_display_flush_ready(disp);
}

void LVGLContext::touch_read_cb(
    lv_indev_t *indev,
    lv_indev_data_t *data)
{
    TouchPoint p = s_touch->read();

    data->state = p.pressed
                      ? LV_INDEV_STATE_PRESSED
                      : LV_INDEV_STATE_RELEASED;

    data->point.x = p.x;
    data->point.y = p.y;
}
