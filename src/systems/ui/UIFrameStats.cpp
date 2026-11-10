#include "UIFrameStats.h"
#if UI_DIAGNOSTICS_ENABLED
#include <Arduino.h>
#include <algorithm>
namespace
{
    uint32_t started, frames, longest, prepareLast, prepareMax;
    uint64_t elapsed;
    void frame(lv_event_t *event)
    {
        if (lv_event_get_code(event) == LV_EVENT_REFR_START) started = micros();
        else if (started)
        {
            const uint32_t duration = micros() - started;
            elapsed += duration;
            longest = std::max(longest, duration);
            ++frames;
            started = 0;
        }
    }
}
void UIFrameStats::attach(lv_display_t *display)
{
    lv_display_add_event_cb(display, frame, LV_EVENT_REFR_START, nullptr);
    lv_display_add_event_cb(display, frame, LV_EVENT_REFR_READY, nullptr);
}
void UIFrameStats::prepared(uint32_t us) { prepareLast = us; prepareMax = std::max(prepareMax, us); }
void UIFrameStats::reset() { started = frames = longest = prepareLast = prepareMax = 0; elapsed = 0; }
void UIFrameStats::print()
{
    Serial.printf("@PERF frames=%lu frame_avg_us=%lu frame_max_us=%lu prepare_last_us=%lu prepare_max_us=%lu\n",
                  (unsigned long)frames, (unsigned long)(frames ? elapsed / frames : 0),
                  (unsigned long)longest, (unsigned long)prepareLast, (unsigned long)prepareMax);
}
#endif
