#pragma once
#include "scenes/game/GameSubview.h"
#include "systems/input/HardwareStatus.h"
#include <lvgl.h>

class HardwareSubview {
  public:
    void create(lv_obj_t* parent, EventCallback callback);
    void destroy();
    void show(const HardwareStatus& report, bool interrupted);
    void hide();
    void translate();

  private:
    lv_obj_t *root = nullptr, *title = nullptr, *details = nullptr, *hint = nullptr;
    lv_obj_t *retry = nullptr, *back = nullptr;
    HardwareStatus report;
    bool interrupted = false;
    EventCallback callback;
    static void onAction(lv_event_t* event);
};
