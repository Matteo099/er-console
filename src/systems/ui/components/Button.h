#pragma once

#include <lvgl.h>

class Button
{
public:
    /**
     * @brief Create a button with optional icon and label
     * @param parent Parent object
     * @param label Button text
     * @param callback Click callback
     * @param userData User data for callback
     * @param width Button width
     * @param height Button height
     * @param align Button alignment
     * @param x X offset
     * @param y Y offset
     * @param icon Optional LVGL symbol (e.g., LV_SYMBOL_PLAY). Pass nullptr for text-only
     */
    static lv_obj_t* create(
        lv_obj_t* parent,
        const char* label,
        lv_event_cb_t callback,
        void* userData,
        lv_coord_t width = 200,
        lv_coord_t height = 42,
        lv_align_t align = LV_ALIGN_CENTER,
        lv_coord_t x = 0,
        lv_coord_t y = 0,
        const char* icon = nullptr)
    {
        lv_obj_t* btn = lv_btn_create(parent);
        lv_obj_set_size(btn, width, height);
        lv_obj_align(btn, align, x, y);
        lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, userData);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_set_style_pad_gap(btn, 6, 0);

        // Create flex container for icon + text
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        if (icon)
        {
            lv_obj_t* icon_label = lv_label_create(btn);
            lv_label_set_text(icon_label, icon);
            lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_18, 0);
        }

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, label);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);

        return btn;
    }
};
