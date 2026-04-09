#pragma once

#include <lvgl.h>
#include <stdint.h>
#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"
#include "scenes/sample/SampleEvents.h"

/**
 * @brief Sample view demonstrating event-driven UI interactions
 *
 * Object Hierarchy & Cleanup Pattern:
 * - Follows Pattern 1 (Simple Root-Based Cleanup)
 * - Root object: `screen` (Screen)
 * - All children owned by screen hierarchy
 * - Cleanup: Delete screen cascades to all children
 *
 * Event System:
 * - Emits PlaySoundEvent when button is clicked
 * - No polling required - events handled immediately by scene
 */
class SampleView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *btn = nullptr;

    static void onButtonClicked(lv_event_t *e);
};
