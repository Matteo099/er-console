#pragma once
#include <lvgl.h>
#include "I18n.h"

// UI-task-only bindings. Labels unregister themselves on LV_EVENT_DELETE.
class LocalizedText {
  public:
    static void bind(lv_obj_t* label, const char* text);
    static void bind(lv_obj_t* label, Text key, const char* prefix = "");
    static void refresh();
};
