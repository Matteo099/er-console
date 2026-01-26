#pragma once
#include <lvgl.h>

#include "drivers/display/DisplayDriver.h"

class BootView {
public:
    BootView(DisplayDriver* display) : display(display) {}

    void create();                          // crea oggetti LVGL
    void update(uint8_t progress, const char* label);  // aggiorna progress bar e label

private:
    DisplayDriver* display;

    lv_obj_t* screen = nullptr;
    lv_obj_t* titleLabel = nullptr;
    lv_obj_t* statusLabel = nullptr;
    lv_obj_t* progressBar = nullptr;
};
