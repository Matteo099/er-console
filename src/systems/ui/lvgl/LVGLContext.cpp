#include "LVGLContext.h"
#include "log/Logger.h"
#include <Arduino.h>
#include "systems/ui/UITheme.h"
#include "systems/ui/UIFrameStats.h"
#include "drivers/display/TouchInputState.h"
#include "config/Console.h"
#include "systems/ui/UIDiagnostics.h"

static const uint32_t LVGL_TICK_PERIOD_MS = 5;

DisplayDriver *LVGLContext::s_display = nullptr;
TouchController *LVGLContext::s_touch = nullptr;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Internal, aligned DMA buffers: render the next strip while SPI sends the first.
DMA_ATTR static uint16_t buf[SCREEN_WIDTH * 40];
DMA_ATTR static uint16_t bufNext[SCREEN_WIDTH * 40];
static lv_draw_buf_t draw_buf, draw_buf_next;

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
    lv_image_cache_resize(psramFound() ? 1024 * 1024 : 128 * 1024, true);
    lv_image_header_cache_resize(16, true);
    lv_tick_set_cb(my_tick);

    // Draw buffer
    lv_draw_buf_init(
        &draw_buf,
        SCREEN_WIDTH,
        40,
        LV_COLOR_FORMAT_RGB565,
        SCREEN_WIDTH * sizeof(uint16_t),
        buf,
        sizeof(buf));
    lv_draw_buf_init(&draw_buf_next, SCREEN_WIDTH, 40, LV_COLOR_FORMAT_RGB565,
                     SCREEN_WIDTH * sizeof(uint16_t), bufNext, sizeof(bufNext));

    // Display
    lv_display_t *disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_flush_wait_cb(disp, flush_wait_cb);
    lv_display_set_draw_buffers(disp, &draw_buf, &draw_buf_next);
    UIFrameStats::attach(disp);

    // Touch
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read_cb);

    // Theme
    UITheme::init(disp);

    Log::info(Log::DISPLAY_TAG, "LVGL Context initialization complete");
}

void LVGLContext::update()
{
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

    // LVGL releases this buffer only after flush_wait_cb confirms DMA completion.
}

void LVGLContext::flush_wait_cb(lv_display_t *) { s_display->waitForFlush(); }

void LVGLContext::touch_read_cb(
    lv_indev_t *indev,
    lv_indev_data_t *data)
{
    static TouchInputState touchState;
    TouchPoint raw;
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    if (!UIDiagnostics::touchSample(raw)) raw = s_touch->read();
#else
    raw = s_touch->read();
#endif
    const TouchPoint p = touchState.update(raw);

    data->state = p.pressed
                      ? LV_INDEV_STATE_PRESSED
                      : LV_INDEV_STATE_RELEASED;

    data->point.x = p.x;
    data->point.y = p.y;
}
