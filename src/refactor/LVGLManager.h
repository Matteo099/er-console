#ifndef LVGL_MANAGER_H
#define LVGL_MANAGER_H

#include <lvgl.h>
#include "LGFX.h"

class LVGLManager {
private:
  LGFX& _display;
  lv_display_t* _disp;
  lv_indev_t* _indev;

public:
  LVGLManager(LGFX& display) : _display(display), _disp(nullptr), _indev(nullptr) {}

  void init() {
    lv_init();
    lv_tick_set_cb(my_tick);

    // Draw buffer
    static lv_color_t buf[SCREEN_WIDTH * 40];
    static lv_draw_buf_t draw_buf;
    lv_draw_buf_init(
      &draw_buf,
      SCREEN_WIDTH,
      40,
      LV_COLOR_FORMAT_RGB565,
      SCREEN_WIDTH * sizeof(lv_color_t),
      buf,
      sizeof(buf)
    );

    // Display
    _disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(_disp, disp_flush_cb);
    lv_display_set_user_data(_disp, &_display);
    lv_display_set_draw_buffers(_disp, &draw_buf, NULL);

    // Touch
    _indev = lv_indev_create();
    lv_indev_set_type(_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(_indev, touch_read_cb);
    lv_indev_set_user_data(_indev, &_display);

    // UI
    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Play Sound");
    lv_obj_center(label);
  }

  static void disp_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
    LGFX* display = static_cast<LGFX*>(lv_display_get_user_data(disp));
    if (!display) return;

    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    display->startWrite();
    display->setAddrWindow(area->x1, area->y1, w, h);
    display->writePixels((lgfx::rgb565_t *)px_map, w * h);
    display->endWrite();

    lv_display_flush_ready(disp);
  }

  static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data) {
    LGFX* display = static_cast<LGFX*>(lv_indev_get_user_data(indev));
    if (!display) return;

    int32_t x, y;
    bool touched = display->getTouch(&x, &y);

    if (!touched) {
      data->state = LV_INDEV_STATE_RELEASED;
    } else {
      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = x;
      data->point.y = y;
    }
  }

  static void btn_event_cb(lv_event_t *e) {
    Serial.println("Button clicked! Playing sound...");
  }

  static uint32_t my_tick(void) {
    return millis();
  }

private:
  static const int SCREEN_WIDTH = 320;
  static const int SCREEN_HEIGHT = 240;
};

#endif // LVGL_MANAGER_H