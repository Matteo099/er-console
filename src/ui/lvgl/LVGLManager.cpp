#include "LVGLManager.h"
#include <Arduino.h>

static const uint32_t LVGL_TICK_PERIOD_MS = 5;

void LVGLManager::init() {
    lv_init();
}

void LVGLManager::tick() {
    lv_tick_inc(LVGL_TICK_PERIOD_MS);
}
