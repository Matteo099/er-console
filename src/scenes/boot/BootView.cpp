#include "BootView.h"
#include "log/Logger.h"

#include "systems/ui/components/BrandHeader.h"

void BootView::create()
{
    // --- Screen ---
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    
    // ---- Brand image (top) ----
    brandImg = BrandHeader::create(screen);

    // --- Logo image (center) ---
    logoImg = lv_img_create(screen);
    lv_img_set_src(logoImg, "S:/logo.png");
    lv_obj_align(logoImg, LV_ALIGN_CENTER, 0, -10);

    // --- Status label (bottom) ---
    statusLabel = lv_label_create(screen);
    lv_label_set_text(statusLabel, "Starting...");
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -28);

    // --- Progress bar (above status) ---
    progressBar = lv_bar_create(screen);
    lv_obj_set_size(progressBar, 220, 14);
    lv_obj_align_to(progressBar, statusLabel, LV_ALIGN_OUT_TOP_MID, 0, -10);

    lv_bar_set_range(progressBar, 0, 100);
    lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(progressBar, lv_color_hex(0x2A2A2A), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progressBar, lv_color_hex(0x00FF88), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progressBar, 7, LV_PART_MAIN | LV_PART_INDICATOR);

    // --- Logo spin animation ---
    lv_anim_init(&logoSpinAnim);
    lv_anim_set_var(&logoSpinAnim, logoImg);
    lv_anim_set_exec_cb(&logoSpinAnim, [](void *obj, int32_t angle)
                        { lv_img_set_angle(static_cast<lv_obj_t *>(obj), angle); });

    lv_anim_set_values(&logoSpinAnim, 0, 3600); // 360° (0.1° units)
    lv_anim_set_time(&logoSpinAnim, 600);
    lv_anim_set_delay(&logoSpinAnim, 1000);        // pause before first spin
    lv_anim_set_repeat_delay(&logoSpinAnim, 1000); // pause between spins
    lv_anim_set_repeat_count(&logoSpinAnim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&logoSpinAnim, lv_anim_path_ease_in_out);

    lv_anim_start(&logoSpinAnim);

    // --- Internal state ---
    currentProgress = 0;
    currentLabel = nullptr;
}

void BootView::createTextOnly()
{
    // --- Screen ---
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // --- Status label (centered initially) ---
    statusLabel = lv_label_create(screen);
    lv_label_set_text(statusLabel, "Starting...");
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0xCCCCCC), 0);
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_14, 0);
    lv_obj_align(statusLabel, LV_ALIGN_CENTER, 0, 20);

    // --- Progress bar ---
    progressBar = lv_bar_create(screen);
    lv_obj_set_size(progressBar, 220, 14);
    lv_obj_align_to(progressBar, statusLabel, LV_ALIGN_OUT_TOP_MID, 0, -12);

    lv_bar_set_range(progressBar, 0, 100);
    lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);

    lv_obj_set_style_bg_color(progressBar, lv_color_hex(0x2A2A2A), LV_PART_MAIN);
    lv_obj_set_style_bg_color(progressBar, lv_color_hex(0x00FF88), LV_PART_INDICATOR);
    lv_obj_set_style_radius(progressBar, 7, LV_PART_MAIN | LV_PART_INDICATOR);

    // --- No images yet ---
    brandImg = nullptr;
    logoImg = nullptr;

    currentProgress = 0;
    currentLabel = nullptr;
}

void BootView::loadImages()
{
    if (!screen)
        return;

    // ---- Brand image (top) ----
    brandImg = BrandHeader::create(screen);


    // ---- Logo image (center) ----
    logoImg = lv_img_create(screen);
    lv_img_set_src(logoImg, "S:/logo.png");
    lv_obj_align(logoImg, LV_ALIGN_CENTER, 0, -10);

    // ---- Reposition text UI to bottom ----
    lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -28);
    lv_obj_align_to(progressBar, statusLabel, LV_ALIGN_OUT_TOP_MID, 0, -10);

    // ---- Logo spin animation ----
    lv_anim_init(&logoSpinAnim);
    lv_anim_set_var(&logoSpinAnim, logoImg);
    lv_anim_set_exec_cb(
        &logoSpinAnim,
        [](void *obj, int32_t angle)
        {
            lv_img_set_angle(static_cast<lv_obj_t *>(obj), angle);
        });

    lv_anim_set_values(&logoSpinAnim, 0, 3600); // 360°
    lv_anim_set_time(&logoSpinAnim, 600);
    lv_anim_set_delay(&logoSpinAnim, 1000);
    lv_anim_set_repeat_delay(&logoSpinAnim, 1000);
    lv_anim_set_repeat_count(&logoSpinAnim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&logoSpinAnim, lv_anim_path_ease_in_out);

    lv_anim_start(&logoSpinAnim);
}

void BootView::setProgress(uint8_t progress, const char *label)
{
    currentProgress = progress;
    currentLabel = label;
}

void BootView::update()
{
    if (!screen)
        return;

    if (progressBar)
    {
        lv_bar_set_value(progressBar, currentProgress, LV_ANIM_ON);
    }

    if (statusLabel && currentLabel)
    {
        lv_label_set_text(statusLabel, currentLabel);
    }
}

void BootView::destroy()
{
    // Stop logo animation (safe even if not running)
    lv_anim_del(logoImg, nullptr);

    if (screen)
    {
        lv_obj_del(screen);
    }

    // Reset all pointers
    screen = nullptr;
    brandImg = nullptr;
    logoImg = nullptr;
    statusLabel = nullptr;
    progressBar = nullptr;

    currentLabel = nullptr;
    currentProgress = 0;
}
