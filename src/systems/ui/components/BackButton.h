#pragma once

#include "systems/ui/localization/I18n.h"
#include <lvgl.h>
#include "Button.h"

class BackButton
{
public:
    static lv_obj_t* create(
        lv_obj_t* parent,
        lv_event_cb_t callback,
        void* userData)
    {
        auto *button = Button::create(parent, tr(Text::Back), callback, userData, 132, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_LEFT);
        UITheme::secondary(button);
        return button;
    }
};
