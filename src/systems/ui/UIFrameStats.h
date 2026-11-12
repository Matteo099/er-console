#pragma once
#include "config/Console.h"
#include <lvgl.h>

// Diagnostics run only on the UI task; snapshots are excluded from frame timing.
class UIFrameStats {
  public:
#if UI_DIAGNOSTICS_ENABLED
    static void attach(lv_display_t* display);
    static void prepared(uint32_t microseconds);
    static void reset();
    static void print();
#else
    static void attach(lv_display_t*) {
    }

    static void prepared(uint32_t) {
    }
#endif
};
