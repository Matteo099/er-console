#pragma once

#include <lvgl.h>

#include "systems/ui/components/Label.h"

class StatisticRow {
  public:
    struct Parts {
        lv_obj_t* container = nullptr;
        lv_obj_t* title = nullptr;
        lv_obj_t* value = nullptr;
    };

    static Parts create(lv_obj_t* parent, const char* title, const char* value = "-") {
        Parts parts;
        if (!parent)
            return parts;
        parts.container = createContainer(parent);
        parts.title = Label::create(parts.container, title, &lv_font_roboto_mono_14,
                                    LV_TEXT_ALIGN_LEFT, LV_SIZE_CONTENT);
        parts.value = Label::create(parts.container, value, &lv_font_roboto_mono_14,
                                    LV_TEXT_ALIGN_RIGHT, LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(parts.container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(parts.container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER);
        UITheme::muted(parts.title);
        UITheme::accent(parts.value);
        return parts;
    }

    static void setValue(Parts& parts, const char* value) {
        if (!parts.value)
            return;
        lv_label_set_text(parts.value, value);
    }

    static void setTitle(Parts& parts, const char* title) {
        if (!parts.title)
            return;
        lv_label_set_text(parts.title, title);
    }

  private:
    static lv_obj_t* createContainer(lv_obj_t* parent) {
        lv_obj_t* row = lv_obj_create(parent);
        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_height(row, 32);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_outline_width(row, 0, 0);
        lv_obj_set_style_shadow_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 4, 0);
        return row;
    }
};
