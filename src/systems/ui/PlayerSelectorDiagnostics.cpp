#include "PlayerSelectorDiagnostics.h"
#include "config/Console.h"

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include <cstring>
#include "components/PlayerSelectorDialog.h"

namespace {
uint32_t testTick = 0;

uint32_t clockTick() {
    return testTick;
}

lv_obj_t* findButton(lv_obj_t* root, const char* icon) {
    for (uint32_t i = 0; i < lv_obj_get_child_count(root); ++i) {
        auto* child = lv_obj_get_child(root, i);
        if (lv_obj_check_type(root, &lv_button_class) &&
            lv_obj_check_type(child, &lv_label_class) &&
            !std::strcmp(lv_label_get_text(child), icon))
            return root;
        if (auto* found = findButton(child, icon))
            return found;
    }
    return nullptr;
}

// Exercise actual LVGL input dispatch and the real dialog without starting a
// game, changing the active hardware display or writing user statistics.
struct SelectorFixture {
    lv_display_t* previous = lv_display_get_default();
    lv_display_t* display = lv_display_create(320, 240);
    lv_indev_t* pointer = nullptr;
    lv_indev_data_t input{};
    PlayerSelectorDialog dialog;
    lv_obj_t *up = nullptr, *down = nullptr, *start = nullptr;
    uint16_t selected = 0;
    lv_tick_get_cb_t previousClock = nullptr;

    SelectorFixture() {
        lv_display_set_default(display);
        lv_display_set_theme(display, lv_display_get_theme(previous));
        auto* screen = lv_display_get_screen_active(display);
        dialog.create(screen, [this](bool confirmed, uint16_t players) {
            if (confirmed)
                selected = players;
        });
        dialog.show();
        lv_obj_update_layout(screen);
        up = findButton(screen, LV_SYMBOL_UP);
        down = findButton(screen, LV_SYMBOL_DOWN);
        start = findButton(screen, LV_SYMBOL_PLAY);
        pointer = lv_indev_create();
        lv_indev_set_type(pointer, LV_INDEV_TYPE_POINTER);
        lv_indev_set_display(pointer, display);
        lv_indev_set_mode(pointer, LV_INDEV_MODE_EVENT);
        lv_indev_set_user_data(pointer, this);
        lv_indev_set_read_cb(pointer, [](lv_indev_t* device, lv_indev_data_t* data) {
            *data = static_cast<SelectorFixture*>(lv_indev_get_user_data(device))->input;
        });
        previousClock = lv_tick_get_cb();
        testTick = lv_tick_get();
        lv_tick_set_cb(clockTick);
    }

    ~SelectorFixture() {
        dialog.clear();
        lv_indev_delete(pointer);
        lv_display_delete(display);
        lv_display_set_default(previous);
        lv_tick_set_cb(previousClock);
    }

    void sample(lv_obj_t* button, bool pressed) {
        lv_obj_update_layout(lv_display_get_screen_active(display));
        lv_area_t bounds;
        lv_obj_get_coords(button, &bounds);
        input.point = {(bounds.x1 + bounds.x2) / 2, (bounds.y1 + bounds.y2) / 2};
        input.state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        lv_indev_read(pointer);
    }

    void tap(lv_obj_t* button) {
        sample(button, true);
        sample(button, false);
    }

    void elapse(uint32_t ms) {
        testTick += ms;
    }

    uint16_t value() {
        lv_obj_send_event(start, LV_EVENT_CLICKED, nullptr);
        return selected;
    }
};
}  // namespace

void runPlayerSelectorDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool okay, const char* name) {
        okay ? ++passed : ++failed;
        Serial.printf("@PLAYERCHECK %s %s\n", okay ? "PASS" : "FAIL", name);
    };
    const bool sound = UIFeedback::soundEnabled();
    UIFeedback::setSoundEnabled(false);
    {
        SelectorFixture f;
        bool increments = true, decrements = true;
        for (unsigned i = 0; i < 10; ++i) {
            f.tap(f.up);
            const auto actual = f.value();
            Serial.printf("@PLAYERTRACE tap=%u value=%u\n", i + 1, unsigned(actual));
            increments &= actual == i + 2;
        }
        check(increments, "every consecutive increment tap changes the count exactly once");
        for (unsigned i = 0; i < 10; ++i) {
            f.tap(f.down);
            decrements &= f.value() == 10 - i;
        }
        check(decrements, "every consecutive decrement tap changes the count exactly once");
        f.tap(f.down);
        check(f.value() == 1, "minimum remains one player");
    }
    {
        SelectorFixture f;
        bool okay = true;
        for (unsigned i = 0; i < 12; ++i) {
            f.tap(f.up);
            okay &= f.value() == 2;
            f.elapse(50);
            f.tap(f.down);
            okay &= f.value() == 1;
            f.elapse(50);
        }
        check(okay, "rapid alternating taps never skip or duplicate a change");
    }
    {
        SelectorFixture f;
        f.sample(f.up, true);
        f.elapse(299);
        f.sample(f.up, true);
        check(f.value() == 1, "hold waits until 300 ms before its first step");
        f.elapse(1);
        f.sample(f.up, true);
        check(f.value() == 2, "hold increments at exactly 300 ms");
        f.elapse(1200);
        f.sample(f.up, false);
        check(f.value() == 6, "1500 ms hold adds exactly five including release boundary");
        f.elapse(900);
        f.sample(f.up, false);
        check(f.value() == 6, "released button stops repeating");
        f.tap(f.up);
        check(f.value() == 7, "tap immediately after a hold increments once");
    }
    {
        SelectorFixture f;
        f.dialog.show(10);
        f.sample(f.down, true);
        bool okay = true;
        for (unsigned i = 0; i < 5; ++i) {
            f.elapse(300);
            f.sample(f.down, true);
            okay &= f.value() == 9 - i;
        }
        f.sample(f.down, false);
        check(okay && f.value() == 5, "decrement repeats once every 300 ms without release step");
    }
    for (auto event : {LV_EVENT_CANCEL, LV_EVENT_PRESS_LOST}) {
        SelectorFixture f;
        f.sample(f.up, true);
        f.elapse(300);
        f.sample(f.up, true);
        lv_obj_send_event(f.up, event, f.pointer);
        f.elapse(1200);
        f.sample(f.up, true);
        f.sample(f.up, false);
        check(f.value() == 2, event == LV_EVENT_CANCEL ? "touch cancel stops repeats and release"
                                                       : "lost press stops repeats and release");
        f.tap(f.up);
        check(f.value() == 3, "new press works immediately after cancellation");
    }
    {
        SelectorFixture f;
        f.sample(f.up, true);
        f.elapse(299);
        // Jitter beyond the common scroll guard's threshold cancels the hold
        // even when the pointer returns to the button before release.
        f.input.point.x += 12;
        lv_indev_read(f.pointer);
        f.elapse(1500);
        f.sample(f.up, true);
        f.sample(f.up, false);
        check(f.value() == 1, "drag out and back cannot repeat or click");
    }
    {
        SelectorFixture f;
        f.sample(f.up, true);
        f.dialog.hide();
        f.elapse(1500);
        f.sample(f.up, false);
        f.dialog.show(7);
        f.tap(f.up);
        check(f.value() == 8, "closing and reopening resets a pending hold");
    }
    {
        SelectorFixture f;
        f.dialog.show(UINT16_MAX - 1);
        f.sample(f.up, true);
        f.elapse(1500);
        f.sample(f.up, true);
        f.sample(f.up, false);
        check(f.value() == UINT16_MAX, "increment hold clamps without overflowing");
        check(!lv_obj_has_state(f.up, LV_STATE_PRESSED), "maximum clears disabled pressed style");
        f.tap(f.down);
        check(f.value() == UINT16_MAX - 1, "decrement works immediately after maximum");
        f.dialog.show(2);
        f.sample(f.down, true);
        f.elapse(1500);
        f.sample(f.down, true);
        f.sample(f.down, false);
        check(f.value() == 1, "decrement hold clamps at minimum one");
        check(!lv_obj_has_state(f.down, LV_STATE_PRESSED), "minimum clears disabled pressed style");
        f.tap(f.up);
        check(f.value() == 2, "increment works immediately after minimum");
    }
    {
        SelectorFixture f;
        testTick = UINT32_MAX - 100;
        f.sample(f.up, true);
        f.elapse(1500);
        f.sample(f.up, false);
        check(f.value() == 6, "hold timing survives millisecond counter wraparound");
    }
    UIFeedback::setSoundEnabled(sound);
    Serial.printf("@PLAYERTEST passed=%u failed=%u\n", passed, failed);
}
#endif
