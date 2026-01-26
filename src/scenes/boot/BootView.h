#pragma once

#include <lvgl.h>
#include <stdint.h>
#include "systems/ui/View.h"

class BootView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    void setProgress(uint8_t progress, const char *label);

    lv_obj_t *getScreen() override { return screen; };

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *titleLabel = nullptr;
    lv_obj_t *statusLabel = nullptr;
    lv_obj_t *progressBar = nullptr;

    uint8_t currentProgress = 0;
    const char *currentLabel = nullptr;
};
