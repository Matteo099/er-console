#pragma once
#include "BrandHeader.h"
#include "SceneLayout.h"
#include "Button.h"
#include "Container.h"
#include "Label.h"
#include <functional>
#include <cstdint>

class PlayerSelectorDialog {
  public:
    void create(lv_obj_t* parent, std::function<void(bool, uint16_t)> selected) {
        callback = std::move(selected);
        root = Container::create(parent, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_LEFT);
        lv_obj_add_flag(root, LV_OBJ_FLAG_FLOATING);
        lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE);
        UITheme::screen(root);
        lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
        auto layout = SceneLayout::create(root, 44, 48, false);
        BrandHeader::create(layout.top, 100, tr(Text::Players));
        Container::setFlex(layout.body, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START,
                           LV_FLEX_ALIGN_START);
        Label::create(layout.body, tr(Text::ChoosePlayers), &lv_font_montserrat_14,
                      LV_TEXT_ALIGN_CENTER, LV_PCT(100));
        auto* row = Container::create(layout.body, LV_PCT(100), 92);
        Container::setFlex(row, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        count = Label::create(row, "1", UITheme::heading(40), LV_TEXT_ALIGN_CENTER, 180);
        auto* arrows = Container::create(row, 48, 92);
        Container::setFlex(arrows);
        lv_obj_set_style_pad_row(arrows, 4, 0);
        up = Button::create(arrows, nullptr, nullptr, nullptr, 44, 44, LV_ALIGN_CENTER, 0, 0,
                            LV_SYMBOL_UP);
        down = Button::create(arrows, nullptr, nullptr, nullptr, 44, 44, LV_ALIGN_CENTER, 0, 0,
                              LV_SYMBOL_DOWN);
        lv_obj_add_event_cb(up, interaction, LV_EVENT_ALL, this);
        lv_obj_add_event_cb(down, interaction, LV_EVENT_ALL, this);
        UITheme::secondary(up);
        UITheme::secondary(down);
        Container::setFlex(layout.footer, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_SPACE_EVENLY,
                           LV_FLEX_ALIGN_CENTER);
        auto* cancel = Button::create(layout.footer, tr(Text::Cancel), cancelled, this, 104, 44);
        UITheme::secondary(cancel);
        Button::create(layout.footer, tr(Text::Start), confirmed, this, 180, 44, LV_ALIGN_CENTER, 0,
                       0, LV_SYMBOL_PLAY);
        hide();
    }

    void show(uint16_t initial = 1) {
        stopPress();
        players = initial ? initial : 1;
        refresh();
        lv_obj_remove_flag(root, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(root);
    }

    void hide() {
        stopPress();
        if (root)
            lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
    }

    bool visible() const {
        return root && !lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN);
    }

    void clear() {
        stopPress();
        // Scene owners release the View before LVGL deletes its screen.
        if (up)
            lv_obj_remove_event_cb(up, interaction);
        if (down)
            lv_obj_remove_event_cb(down, interaction);
        root = count = up = down = nullptr;
        callback = nullptr;
    }

  private:
    void refresh() {
        char text[8];
        lv_snprintf(text, sizeof(text), "%u", unsigned(players));
        Label::setText(count, text);
        lv_obj_set_state(down, LV_STATE_DISABLED, players == 1);
        lv_obj_set_state(up, LV_STATE_DISABLED, players == UINT16_MAX);
        // LVGL omits RELEASED for a button disabled during a hold. Clear its
        // pressed style now, so it is not stuck when the opposite arrow enables it.
        if (players == 1)
            lv_obj_remove_state(down, LV_STATE_PRESSED);
        if (players == UINT16_MAX)
            lv_obj_remove_state(up, LV_STATE_PRESSED);
    }

    void adjust(lv_obj_t* button, uint32_t steps) {
        const uint32_t available = button == up ? UINT16_MAX - players : players - 1;
        if (steps > available)
            steps = available;
        if (!steps)
            return;
        players = button == up ? players + steps : players - steps;
        refresh();
    }

    void stopPress() {
        held = nullptr;
        device = nullptr;
        repeated = true;  // A cancelled gesture cannot become a tap on release.
    }

    void repeat(lv_event_t* event) {
        if (!held)
            return;
        if (device && lv_indev_get_type(device) == LV_INDEV_TYPE_POINTER) {
            lv_point_t point;
            lv_indev_get_point(device, &point);
            lv_area_t bounds;
            lv_obj_get_coords(held, &bounds);
            const int32_t dx = point.x - origin.x, dy = point.y - origin.y;
            if (device != lv_event_get_indev(event) || lv_indev_get_scroll_obj(device) ||
                dx * dx + dy * dy >
                    ButtonClickGuard::MOVEMENT_LIMIT * ButtonClickGuard::MOVEMENT_LIMIT ||
                !lv_area_is_point_on(&bounds, &point, 0)) {
                stopPress();
                return;
            }
        }
        // PRESSING is delivered by LVGL's input timer. Count elapsed intervals
        // rather than frames, including an interval ending exactly on release.
        const uint32_t steps = lv_tick_elaps(repeatedAt) / REPEAT_MS;
        if (steps) {
            repeatedAt += steps * REPEAT_MS;
            repeated = true;
            adjust(held, steps);
        }
    }

    static void interaction(lv_event_t* event) {
        auto* self = static_cast<PlayerSelectorDialog*>(lv_event_get_user_data(event));
        auto* button = lv_event_get_current_target_obj(event);
        if (lv_event_get_target(event) != button)
            return;
        switch (lv_event_get_code(event)) {
            case LV_EVENT_PRESSED:
                self->held = button;
                self->device = lv_event_get_indev(event);
                if (self->device && lv_indev_get_type(self->device) == LV_INDEV_TYPE_POINTER)
                    lv_indev_get_point(self->device, &self->origin);
                self->repeatedAt = lv_tick_get();
                self->repeated = false;
                break;
            case LV_EVENT_PRESSING:
                self->repeat(event);
                break;
            case LV_EVENT_RELEASED:
                self->repeat(event);
                self->held = nullptr;
                self->device = nullptr;
                break;
            case LV_EVENT_CLICKED:
                // CLICKED fires after both a tap and a hold. A hold already
                // applied its steps; the trailing click must not add another.
                if (!lv_event_get_indev(event) || !self->repeated)
                    self->adjust(button, 1);
                self->stopPress();
                break;
            case LV_EVENT_PRESS_LOST:
            case LV_EVENT_CANCEL:
            case LV_EVENT_DELETE:
                self->stopPress();
                break;
            default:
                break;
        }
    }

    static void confirmed(lv_event_t* event) {
        auto* self = static_cast<PlayerSelectorDialog*>(lv_event_get_user_data(event));
        if (self->callback)
            self->callback(true, self->players);
    }

    static void cancelled(lv_event_t* event) {
        auto* self = static_cast<PlayerSelectorDialog*>(lv_event_get_user_data(event));
        if (self->callback)
            self->callback(false, self->players);
    }

    lv_obj_t *root = nullptr, *count = nullptr, *up = nullptr, *down = nullptr;
    static constexpr uint32_t REPEAT_MS = 300;
    lv_obj_t* held = nullptr;
    lv_indev_t* device = nullptr;
    lv_point_t origin{};
    uint32_t repeatedAt = 0;
    bool repeated = true;
    uint16_t players = 1;
    std::function<void(bool, uint16_t)> callback;
};
