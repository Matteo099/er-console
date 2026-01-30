#include "BackButton.h"

lv_obj_t* BackButton::create(
    lv_obj_t* parent,
    lv_event_cb_t callback,
    void* userData)
{
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 90, 36);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -12);

    lv_obj_add_event_cb(
        btn,
        callback,
        LV_EVENT_CLICKED,
        userData);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "Back");
    lv_obj_center(lbl);

    return btn;
}
