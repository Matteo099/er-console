#pragma once
#include <lvgl.h>
#include "devices/display/Display.h"

class BootView {
public:
    BootView(Display* display) : display(display) {}

    void create();                          // crea oggetti LVGL
    void update(uint8_t progress, const char* label);  // aggiorna progress bar e label

private:
    Display* display;

    lv_obj_t* screen = nullptr;
    lv_obj_t* titleLabel = nullptr;
    lv_obj_t* statusLabel = nullptr;
    lv_obj_t* progressBar = nullptr;
};
