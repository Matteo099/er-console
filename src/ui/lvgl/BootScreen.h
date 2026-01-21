#pragma once
#include <lvgl.h>

class BootScreen {
public:
    void create();
    void update(uint8_t progress, const char* label);

private:
    lv_obj_t* screen = nullptr;
    lv_obj_t* titleLabel = nullptr;
    lv_obj_t* statusLabel = nullptr;
    lv_obj_t* progressBar = nullptr;
};
