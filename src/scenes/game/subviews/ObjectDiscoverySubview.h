#pragma once
#include "GameObjectSubview.h"

// Time-driven, small-area animation: no LVGL timers/callbacks outlive the view.
class ObjectDiscoverySubview {
  public:
    static constexpr uint32_t RevealMs = 650, FlightMs = 500, DurationMs = RevealMs + FlightMs;
    void create(lv_obj_t* parent);
    void show(const GameObjectSubview::ObjectData& object, lv_obj_t* destination, uint32_t now);
    bool update(uint32_t now);  // true only on landing
    void hide();
    void destroy();

    bool visible() const {
        return active;
    }

    void translate();

  private:
    lv_obj_t *root = nullptr, *item = nullptr, *caption = nullptr, *title = nullptr;
    lv_coord_t startX = 0, startY = 0, targetX = 0, targetY = 0;
    uint32_t started = 0, rendered = 0;
    bool active = false;
};
