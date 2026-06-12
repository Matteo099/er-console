#pragma once

#include <lvgl.h>
#include <stdint.h>
#include "systems/ui/View.h"
#include "systems/ui/components/Spinner.h"

/**
 * @brief Boot/splash screen view with progress indication
 * 
 * Object Hierarchy & Cleanup Pattern:
 * - Follows Pattern 1 (Simple Root-Based Cleanup)
 * - Root object: `screen` (Screen)
 * - Spinner animations auto-managed by Spinner component
 * - Cleanup: Delete screen cascades to all children
 */
class BootView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    void setProgress(uint8_t progress, const char *label);
    void loadImages();
    void createTextOnly();

    lv_obj_t *getScreen() override { return screen; };

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *topRow = nullptr;
    lv_obj_t *middleRow = nullptr;
    lv_obj_t *bottomRow = nullptr;
    lv_obj_t *brandImg = nullptr;
    lv_obj_t *logoImg = nullptr;
    lv_obj_t *statusLabel = nullptr;
    lv_obj_t *progressBar = nullptr;

    // Animation management handled by Spinner component
    // (no manual lv_anim_t needed)

    uint8_t currentProgress = 0;
    const char *currentLabel = nullptr;
};
