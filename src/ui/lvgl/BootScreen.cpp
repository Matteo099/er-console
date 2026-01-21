#include "BootScreen.h"

void BootScreen::create() {

    screen = lv_obj_create(NULL);
    lv_scr_load(screen);

    titleLabel = lv_label_create(screen);
    lv_label_set_text(titleLabel, "Escape Room Console");
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 20);

    statusLabel = lv_label_create(screen);
    lv_label_set_text(statusLabel, "Starting...");
    lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 0, 60);

    progressBar = lv_bar_create(screen);
    lv_obj_set_size(progressBar, 220, 20);
    lv_obj_align(progressBar, LV_ALIGN_TOP_MID, 0, 110);
    lv_bar_set_range(progressBar, 0, 100);
    lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);
}

void BootScreen::update(uint8_t progress, const char* label) {
    lv_label_set_text(statusLabel, label);
    lv_bar_set_value(progressBar, progress, LV_ANIM_OFF);
}
