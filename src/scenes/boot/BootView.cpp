#include "BootView.h"
#include "log/Logger.h"

#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/ImageView.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/ProgressBar.h"
#include "systems/ui/components/Screen.h"
#include "systems/ui/components/Align.h"

void BootView::create()
{
    screen = Screen::create();
    brandImg = BrandHeader::create(screen);
    logoImg = ImageView::create(screen, "S:/logo.png", LV_ALIGN_CENTER, 0, -10);

    statusLabel = Label::create(screen, "Starting...", &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);
    Label::setTextColor(statusLabel, lv_color_hex(0xCCCCCC));
    Align::setPosition(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -28);

    progressBar = ProgressBar::create(screen);
    Align::alignTo(progressBar, statusLabel, LV_ALIGN_OUT_TOP_MID, 0, -10);

    lv_anim_init(&logoSpinAnim);
    lv_anim_set_var(&logoSpinAnim, logoImg);
    lv_anim_set_exec_cb(&logoSpinAnim, [](void *obj, int32_t angle)
                        { lv_img_set_angle(static_cast<lv_obj_t *>(obj), angle); });

    lv_anim_set_values(&logoSpinAnim, 0, 3600);
    lv_anim_set_time(&logoSpinAnim, 600);
    lv_anim_set_delay(&logoSpinAnim, 1000);
    lv_anim_set_repeat_delay(&logoSpinAnim, 1000);
    lv_anim_set_repeat_count(&logoSpinAnim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&logoSpinAnim, lv_anim_path_ease_in_out);

    lv_anim_start(&logoSpinAnim);

    currentProgress = 0;
    currentLabel = nullptr;
}

void BootView::createTextOnly()
{
    screen = Screen::create();

    statusLabel = Label::create(screen, "Starting...", &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);
    Label::setTextColor(statusLabel, lv_color_hex(0xCCCCCC));
    Align::setPosition(statusLabel, LV_ALIGN_CENTER, 0, 20);

    progressBar = ProgressBar::create(screen);
    Align::alignTo(progressBar, statusLabel, LV_ALIGN_OUT_TOP_MID, 0, -12);

    brandImg = nullptr;
    logoImg = nullptr;

    currentProgress = 0;
    currentLabel = nullptr;
}

void BootView::loadImages()
{
    if (!screen)
        return;

    brandImg = BrandHeader::create(screen);
    logoImg = ImageView::create(screen, "S:/logo.png", LV_ALIGN_CENTER, 0, -10);

    Align::setPosition(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -28);
    Align::alignTo(progressBar, statusLabel, LV_ALIGN_OUT_TOP_MID, 0, -10);

    lv_anim_init(&logoSpinAnim);
    lv_anim_set_var(&logoSpinAnim, logoImg);
    lv_anim_set_exec_cb(&logoSpinAnim, [](void *obj, int32_t angle)
                        { lv_img_set_angle(static_cast<lv_obj_t *>(obj), angle); });

    lv_anim_set_values(&logoSpinAnim, 0, 3600);
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
        ProgressBar::setValue(progressBar, currentProgress, LV_ANIM_ON);
    }

    if (statusLabel && currentLabel)
    {
        lv_label_set_text(statusLabel, currentLabel);
    }
}

void BootView::destroy()
{
    // Clean up tracker (for any dynamic objects)
    getObjectTracker().cleanup();

    // Stop logo animation before object deletion
    lv_anim_del(logoImg, nullptr);

    // Delete root (cascades to all children)
    UICleanup::deleteRoot(screen);

    // Nullify remaining pointers
    brandImg = nullptr;
    logoImg = nullptr;
    statusLabel = nullptr;
    progressBar = nullptr;
    currentLabel = nullptr;
    currentProgress = 0;
}
