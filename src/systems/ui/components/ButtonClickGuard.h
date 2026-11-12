#pragma once
#include <lvgl.h>

// Pointer intent is shared by every Button::create caller, independently of
// the pointer driver (resistive touch, mouse or trackpad).
class ButtonClickGuard {
  public:
    static constexpr int32_t MOVEMENT_LIMIT = 8;
    static void attach(lv_obj_t* button);

  private:
    struct Press {
        lv_point_t origin{};
        lv_indev_t* device = nullptr;
        bool armed = false;
        bool cancelled = false;
    };

    static void onEvent(lv_event_t* event);
};
