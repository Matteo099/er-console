#pragma once
#include "TouchController.h"

// Coordinates are meaningful only while the resistive panel is pressed.
// LVGL also uses the release position to finish slider/keyboard interactions.
class TouchInputState {
  public:
    TouchPoint update(TouchPoint sample) {
        if (sample.pressed)
            last = sample;
        last.pressed = sample.pressed;
        return last;
    }

  private:
    TouchPoint last{0, 0, false};
};
