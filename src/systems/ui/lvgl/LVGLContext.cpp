#include "LVGLContext.h"
#include "log/Logger.h"
#include <Arduino.h>

static const uint32_t LVGL_TICK_PERIOD_MS = 5;

DisplayDriver *LVGLContext::s_display = nullptr;
TouchController *LVGLContext::s_touch = nullptr;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

static lv_color_t buf[SCREEN_WIDTH * 40];
static lv_draw_buf_t draw_buf;

uint32_t my_tick(void)
{
    return millis();
}

static void btn_event_cb(lv_event_t *e)
{
    Serial.println("Button clicked!");
}

void LVGLContext::init(DisplayDriver *displayDrv, TouchController *touchDrv)
{
    s_display = displayDrv;
    s_touch = touchDrv;

    Log::info(Log::DISPLAY_TAG, "Initializing LVGL Context");

    lv_init();
    lv_tick_set_cb(my_tick);

    // Draw buffer
    lv_draw_buf_init(
        &draw_buf,
        SCREEN_WIDTH,
        40,
        LV_COLOR_FORMAT_RGB565,
        SCREEN_WIDTH * sizeof(lv_color_t),
        buf,
        sizeof(buf));

    // Display
    lv_display_t *disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_draw_buffers(disp, &draw_buf, NULL);

    // Touch
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    // UI
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Play Sound");
    lv_obj_center(label);

    Log::info(Log::DISPLAY_TAG, "LVGL Context initialization complete");
}

void LVGLContext::update()
{
    // lv_tick_inc(LVGL_TICK_PERIOD_MS);
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
        px_map);

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
