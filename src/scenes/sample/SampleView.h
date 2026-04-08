#pragma once

#include <lvgl.h>
#include <stdint.h>
#include "systems/ui/View.h"

class SampleView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };
    bool consumeAction();

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *btn = nullptr;
    lv_obj_t *label = nullptr;

    bool playSound = false;

    static void onButtonClicked(lv_event_t *e);
};
