#include "ScrollDiagnostics.h"
#include "config/Console.h"

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include "components/Button.h"

namespace {
// Drive LVGL's actual pointer/scroll processing, not synthetic CLICKED
// events. A separate display prevents navigation or preference changes.
struct PointerFixture {
    lv_display_t* previous = lv_display_get_default();
    lv_display_t* display = lv_display_create(320, 240);
    lv_indev_t* pointer = nullptr;
    lv_obj_t *list = nullptr, *button = nullptr, *second = nullptr;
    lv_indev_data_t input{};
    unsigned clicks = 0, otherClicks = 0, feedback = 0;

    explicit PointerFixture(bool legacy = false) {
        lv_display_set_default(display);
        auto* screen = lv_display_get_screen_active(display);
        lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        list = lv_obj_create(screen);
        lv_obj_set_pos(list, 20, 20);
        lv_obj_set_size(list, 280, 200);
        lv_obj_set_style_pad_all(list, 0, 0);
        lv_obj_set_style_border_width(list, 0, 0);
        lv_obj_set_scroll_dir(list, LV_DIR_VER);
        lv_obj_remove_flag(list, static_cast<lv_obj_flag_t>(LV_OBJ_FLAG_SCROLL_ELASTIC |
                                                            LV_OBJ_FLAG_SCROLL_MOMENTUM));
        auto* nested = lv_obj_create(list);
        lv_obj_set_pos(nested, 0, 0);
        lv_obj_set_size(nested, 260, 480);
        lv_obj_set_style_pad_all(nested, 0, 0);
        lv_obj_set_style_border_width(nested, 0, 0);
        lv_obj_remove_flag(nested, LV_OBJ_FLAG_SCROLLABLE);
        if (legacy) {
            button = lv_button_create(nested);
            lv_obj_set_pos(button, 10, 20);
            lv_obj_set_size(button, 240, 60);
            lv_obj_add_event_cb(button, count, LV_EVENT_CLICKED, &clicks);
        } else
            button =
                Button::create(nested, "A", count, &clicks, 240, 60, LV_ALIGN_TOP_LEFT, 10, 20);
        second =
            Button::create(nested, "B", count, &otherClicks, 240, 60, LV_ALIGN_TOP_LEFT, 10, 100);
        lv_obj_add_event_cb(button, count, LV_EVENT_CLICKED, &feedback);
        lv_obj_update_layout(screen);
        pointer = lv_indev_create();
        lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
        lv_indev_set_display(pointer, display);
        lv_indev_set_mode(pointer, LV_INDEV_MODE_EVENT);
        lv_indev_set_user_data(pointer, this);
        lv_indev_set_read_cb(pointer, read);
    }

    ~PointerFixture() {
        lv_indev_delete(pointer);
        lv_display_delete(display);
        lv_display_set_default(previous);
    }

    static void count(lv_event_t* event) {
        ++*static_cast<unsigned*>(lv_event_get_user_data(event));
    }

    static void read(lv_indev_t* device, lv_indev_data_t* data) {
        *data = static_cast<PointerFixture*>(lv_indev_get_user_data(device))->input;
    }

    void sample(int x, int y, bool pressed, int wheel = 0) {
        input.point = {x, y};
        input.state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        input.enc_diff = wheel;
        lv_indev_read(pointer);
    }

    bool silent() const {
        return !clicks && !otherClicks && !feedback;
    }
};
}  // namespace

void runScrollDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool okay, const char* name) {
        okay ? ++passed : ++failed;
        Serial.printf("@SCROLLCHECK %s %s\n", okay ? "PASS" : "FAIL", name);
    };
    const bool sound = UIFeedback::soundEnabled();
    UIFeedback::setSoundEnabled(false);
    {
        PointerFixture f(true);
        f.sample(100, 65, true);
        f.sample(130, 65, true);
        f.sample(130, 65, false);
        check(f.clicks == 1, "unguarded LVGL button reproduces accidental drag click");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        check(f.silent(), "press alone never activates");
        f.sample(100, 65, false);
        check(f.clicks == 1 && f.feedback == 1, "intentional touch or mouse tap activates once");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        f.sample(103, 68, true);
        f.sample(103, 68, false);
        check(f.clicks == 1, "small finger jitter still permits a tap");
    }
    {
        PointerFixture f;
        f.sample(100, 85, true);
        f.sample(100, 70, true);
        f.sample(100, 55, true);
        f.sample(100, 55, false);
        check(lv_obj_get_scroll_y(f.list) > 0,
              "drag over nested button actually scrolls its parent");
        check(f.silent(), "touch scroll does not invoke action or feedback");
    }
    {
        PointerFixture f;
        f.sample(100, 60, true);
        f.sample(100, 85, true);
        f.sample(100, 85, false);
        check(lv_obj_get_scroll_y(f.list) == 0 && f.silent(),
              "drag at upper scroll boundary cannot click");
    }
    {
        PointerFixture f;
        lv_obj_set_pos(f.button, 10, 380);
        lv_obj_update_layout(lv_display_get_screen_active(f.display));
        lv_obj_scroll_to_y(f.list, 280, LV_ANIM_OFF);
        f.sample(100, 160, true);
        f.sample(100, 135, true);
        f.sample(100, 135, false);
        check(lv_obj_get_scroll_y(f.list) == 280 && f.silent(),
              "drag at lower scroll boundary cannot click");
    }
    {
        PointerFixture f;
        lv_obj_set_scroll_dir(f.list, LV_DIR_HOR);
        lv_obj_set_size(lv_obj_get_parent(f.button), 480, 180);
        lv_obj_update_layout(lv_display_get_screen_active(f.display));
        f.sample(160, 65, true);
        f.sample(130, 65, true);
        f.sample(130, 65, false);
        check(lv_obj_get_scroll_x(f.list) > 0 && f.silent(),
              "horizontal sections scroll without activating buttons");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        f.sample(130, 65, true);
        f.sample(130, 65, false);
        check(f.silent(), "mouse drag on non-scrolling axis cannot click");
        f.sample(100, 65, true);
        f.sample(100, 65, false);
        check(f.clicks == 1, "fresh tap works immediately after a cancelled drag");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        for (int x = 101; x <= 116; ++x)
            f.sample(x, 65, true);
        f.sample(116, 65, false);
        check(f.silent(), "slow drag uses full displacement rather than per-frame delta");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        f.sample(130, 65, true);
        f.sample(100, 65, true);
        f.sample(100, 65, false);
        check(f.silent(), "drag returning to its origin stays cancelled");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        f.sample(295, 65, true);
        f.sample(100, 65, true);
        f.sample(100, 65, false);
        check(f.silent(), "leaving and re-entering button cannot activate it");
    }
    {
        PointerFixture f;
        f.sample(100, 28, true);
        f.sample(100, 65, true);
        f.sample(100, 65, false);
        check(f.silent(), "gesture starting outside a button cannot become a tap");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        f.sample(100, 145, true);
        f.sample(100, 145, false);
        check(f.silent(), "crossing another button cannot activate either button");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        f.sample(100, 65, true, -2);
        check(lv_indev_get_scroll_obj(f.pointer) == f.list,
              "mouse wheel or trackpad delta is recognized as scrolling");
        f.sample(100, 65, false);
        check(f.silent(), "stationary pointer release during wheel scrolling cannot click");
    }
    {
        PointerFixture f;
        // LVGL 9.4 initializes the pointer's scroll limits on its first drag.
        // Exercise wheel movement after that native initialization as well.
        f.sample(100, 85, true);
        f.sample(100, 55, true);
        f.sample(100, 55, false);
        lv_obj_scroll_to_y(f.list, 0, LV_ANIM_OFF);
        f.sample(100, 65, true);
        f.sample(100, 65, true, -2);
        f.sample(100, 65, false);
        check(lv_obj_get_scroll_y(f.list) > 0 && f.silent(),
              "initialized wheel scrolling moves content without clicks");
    }
    {
        PointerFixture f;
        f.sample(100, 65, true);
        f.sample(100, 65, true, 2);
        f.sample(100, 65, false);
        check(f.silent(), "wheel scrolling at the list boundary cannot become a click");
    }
    {
        PointerFixture f;
        lv_obj_add_state(f.button, LV_STATE_DISABLED);
        f.sample(100, 65, true);
        f.sample(100, 65, false);
        check(f.silent(), "disabled buttons remain inactive");
    }
    {
        PointerFixture f;
        auto* group = lv_group_create();
        auto* keyboard = lv_indev_create();
        lv_indev_set_type(keyboard, LV_INDEV_TYPE_KEYPAD);
        lv_indev_set_display(keyboard, f.display);
        lv_indev_set_mode(keyboard, LV_INDEV_MODE_EVENT);
        lv_indev_set_group(keyboard, group);
        lv_group_add_obj(group, f.button);
        lv_group_focus_obj(f.button);
        lv_indev_set_user_data(keyboard, &f);
        lv_indev_set_read_cb(keyboard, PointerFixture::read);
        f.input.key = LV_KEY_ENTER;
        f.input.state = LV_INDEV_STATE_PRESSED;
        lv_indev_read(keyboard);
        f.input.state = LV_INDEV_STATE_RELEASED;
        lv_indev_read(keyboard);
        check(f.clicks == 1, "keyboard Enter retains intentional activation");
        lv_indev_delete(keyboard);
        lv_group_delete(group);
    }
    {
        PointerFixture f;
        lv_obj_send_event(f.button, LV_EVENT_CLICKED, nullptr);
        check(f.clicks == 1, "explicit application clicks retain their semantics");
    }
    UIFeedback::setSoundEnabled(sound);
    Serial.printf("@SCROLLTEST passed=%u failed=%u\n", passed, failed);
}
#endif
