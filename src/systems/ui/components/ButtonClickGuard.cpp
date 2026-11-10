#include "ButtonClickGuard.h"

void ButtonClickGuard::attach(lv_obj_t *button)
{
    // Run before the widget and application callbacks, including click audio.
    // LVGL owns the callback; its DELETE event releases the associated state.
    lv_obj_add_event_cb(button, onEvent, static_cast<lv_event_code_t>(LV_EVENT_ALL | LV_EVENT_PREPROCESS), new Press{});
}

void ButtonClickGuard::onEvent(lv_event_t *event)
{
    if (lv_event_get_target(event) != lv_event_get_current_target(event)) return;
    auto *press = static_cast<Press *>(lv_event_get_user_data(event));
    const auto code = lv_event_get_code(event);
    if (code == LV_EVENT_DELETE) { delete press; return; }
    if (code == LV_EVENT_PRESS_LOST || code == LV_EVENT_CANCEL)
    {
        press->cancelled = true;
        return;
    }
    if (code != LV_EVENT_PRESSED && code != LV_EVENT_PRESSING &&
        code != LV_EVENT_RELEASED && code != LV_EVENT_CLICKED) return;

    auto *device = lv_event_get_indev(event);
    // Keyboard/encoder activation and explicit programmatic clicks keep their
    // normal semantics. Only a physical pointer gesture needs a matching press.
    if (!device || lv_indev_get_type(device) != LV_INDEV_TYPE_POINTER) return;
    lv_point_t point;
    lv_indev_get_point(device, &point);
    if (code == LV_EVENT_PRESSED)
    {
        press->origin = point;
        press->device = device;
        press->armed = true;
        press->cancelled = lv_indev_get_scroll_obj(device) != nullptr;
    }
    else
    {
        const int32_t dx = point.x - press->origin.x;
        const int32_t dy = point.y - press->origin.y;
        // Cancellation is latched until a new press, including out-and-back
        // drags, slow movement, and drags at a scroll boundary.
        if (device != press->device || dx * dx + dy * dy > MOVEMENT_LIMIT * MOVEMENT_LIMIT ||
            lv_indev_get_scroll_obj(device)) press->cancelled = true;
    }
    if (code == LV_EVENT_CLICKED)
    {
        lv_area_t bounds;
        lv_obj_get_coords(lv_event_get_current_target_obj(event), &bounds);
        const bool accept = press->armed && !press->cancelled && lv_area_is_point_on(&bounds, &point, 0);
        press->armed = false;
        if (!accept)
        {
            lv_event_stop_processing(event);
            lv_event_stop_bubbling(event);
        }
    }
}
