#include "BootView.h"


void BootView::create()
{
    // Create screen (DO NOT load it here)
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    // Background
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // Title
    titleLabel = lv_label_create(screen);
    lv_label_set_text(titleLabel, "Escape Room Console");
    lv_obj_set_style_text_color(titleLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 20);

    // Status label
    statusLabel = lv_label_create(screen);
    lv_label_set_text(statusLabel, "Starting...");
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 0, 70);

    // Progress bar
    progressBar = lv_bar_create(screen);
    lv_obj_set_size(progressBar, 220, 16);
    lv_obj_align(progressBar, LV_ALIGN_TOP_MID, 0, 120);

    lv_bar_set_range(progressBar, 0, 100);
    lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(progressBar, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progressBar, lv_color_hex(0x00FF88), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progressBar, 8, LV_PART_MAIN | LV_PART_INDICATOR);

    // Internal state
    currentProgress = 0;
    currentLabel = nullptr;
}

void BootView::setProgress(uint8_t progress, const char* label)
{
    currentProgress = progress;
    currentLabel = label;
}

void BootView::update()
{
    if (!screen) return;

    if (progressBar) {
        lv_bar_set_value(progressBar, currentProgress, LV_ANIM_ON);
    }

    if (statusLabel && currentLabel) {
        lv_label_set_text(statusLabel, currentLabel);
    }
}

void BootView::destroy()
{
    if (screen) {
        lv_obj_del(screen);
        screen = nullptr;
        titleLabel = nullptr;
        statusLabel = nullptr;
        progressBar = nullptr;
    }
}
