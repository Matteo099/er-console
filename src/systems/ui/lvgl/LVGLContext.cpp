#include "LVGLSystem.h"

static const uint32_t LVGL_TICK_PERIOD_MS = 5;

void LVGLContext::init() {
    lv_init();
}

void LVGLContext::update() {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}
