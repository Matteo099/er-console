#pragma once
#include <lvgl.h>
#include "systems/ui/UITheme.h"

// ESCAPERIOM logo, drawn entirely in code (no image assets, no heap).
// Design space: mark = 120x120, lockup = 760x160. Everything scales to the object size.
// Usage:
// EscaperiomLogo::createMark(header, 32);               // small header icon
// EscaperiomLogo::createLockup(screen, 240, true);      // full logo on a dark background
class EscaperiomLogo {
  public:
    // Emblem only (square). Use for small sizes.
    static lv_obj_t* createMark(lv_obj_t* parent, int size, bool onDark = false) {
        return make(parent, size, size, MODE_MARK | (onDark ? FLAG_DARK : 0));
    }

    // Emblem + wordmark (aspect 760:160). Recommended width >= 160 px.
    static lv_obj_t* createLockup(lv_obj_t* parent, int width, bool onDark = false) {
        return make(parent, width, width * 160 / 760, MODE_LOCKUP | (onDark ? FLAG_DARK : 0));
    }

  private:
    enum : intptr_t { MODE_MARK = 0, MODE_LOCKUP = 1, FLAG_DARK = 2 };

    static constexpr int SW = 11;  // wordmark stroke (design units)
    static constexpr int T = 56, M = 80, B = 104, W = 34;  // letter top / middle / bottom / width

    static lv_obj_t* make(lv_obj_t* parent, int w, int h, intptr_t flags) {
        auto* obj = lv_obj_create(parent);
        lv_obj_remove_style_all(obj);
        lv_obj_set_size(obj, w, h);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_event_cb(obj, draw, LV_EVENT_DRAW_MAIN, (void*)flags);
        return obj;
    }

    struct Ctx {
        lv_layer_t* layer;
        lv_area_t b;
        int designW;
        int w;
        uint32_t mark, core, ink;
        int ox, oy;  // design-space offset applied to emblem

        int X(int v) const {
            return b.x1 + v * w / designW;
        }

        int Y(int v) const {
            return b.y1 + v * w / designW;
        }

        void rect(int x1, int y1, int x2, int y2, uint32_t color) const {
            lv_draw_rect_dsc_t d;
            lv_draw_rect_dsc_init(&d);
            d.bg_color = lv_color_hex(color);
            d.bg_opa = LV_OPA_COVER;
            lv_area_t a = {X(x1), Y(y1), X(x2) - 1, Y(y2) - 1};
            if (a.x2 < a.x1)
                a.x2 = a.x1;  // never vanish at tiny sizes
            if (a.y2 < a.y1)
                a.y2 = a.y1;
            lv_draw_rect(layer, &d, &a);
        }

        void line(int x1, int y1, int x2, int y2, uint32_t color) const {
            lv_draw_line_dsc_t d;
            lv_draw_line_dsc_init(&d);
            d.p1 = {(lv_value_precise_t)X(x1), (lv_value_precise_t)Y(y1)};
            d.p2 = {(lv_value_precise_t)X(x2), (lv_value_precise_t)Y(y2)};
            d.width = LV_MAX(1, SW * w / designW);
            d.color = lv_color_hex(color);
            d.round_start = d.round_end = 0;
            lv_draw_line(layer, &d);
        }

        // stroke helpers (centerline based)
        void H(int xa, int xb, int yc, uint32_t c) const {
            rect(xa, yc - SW / 2, xb, yc + SW / 2, c);
        }

        void V(int xc, int ya, int yb, uint32_t c) const {
            rect(xc - SW / 2, ya, xc + SW / 2, yb, c);
        }
    };

    static void emblem(const Ctx& c) {
        const int x = c.ox, y = c.oy;
        // outer walls, open top-right corner (exit)
        c.rect(x + 0, y + 0, x + 75, y + 10, c.mark);
        c.rect(x + 0, y + 0, x + 10, y + 120, c.mark);
        c.rect(x + 0, y + 110, x + 120, y + 120, c.mark);
        c.rect(x + 110, y + 55, x + 120, y + 120, c.mark);
        // inner ring, offset opening
        c.rect(x + 80, y + 55, x + 90, y + 90, c.mark);
        c.rect(x + 30, y + 80, x + 90, y + 90, c.mark);
        c.rect(x + 30, y + 30, x + 40, y + 90, c.mark);
        c.rect(x + 30, y + 30, x + 65, y + 40, c.mark);
        // modular connector + puzzle core
        c.rect(x + 55, y + 80, x + 65, y + 110, c.mark);
        c.rect(x + 50, y + 50, x + 70, y + 70, c.core);
    }

    static void wordmark(const Ctx& c, uint32_t ink) {
        int x0;
        // E
        x0 = 190;
        c.V(x0, T - 6, B + 6, ink);
        c.H(x0, x0 + W, T, ink);
        c.H(x0, x0 + W, B, ink);
        c.H(x0, x0 + 28, M, ink);
        // S
        x0 = 247;
        c.H(x0 - 6, x0 + W, T, ink);
        c.V(x0, T - 6, M + 6, ink);
        c.H(x0 - 6, x0 + W + 6, M, ink);
        c.V(x0 + W, M - 6, B + 6, ink);
        c.H(x0 - 6, x0 + W, B, ink);
        // C
        x0 = 304;
        c.H(x0 - 6, x0 + W, T, ink);
        c.V(x0, T - 6, B + 6, ink);
        c.H(x0, x0 + W, B, ink);
        // A (flat apex)
        x0 = 361;
        c.rect(x0 + 13, T - 6, x0 + 21, T + 6, ink);
        c.line(x0, B + 2, x0 + 13, T, ink);
        c.line(x0 + 34, B + 2, x0 + 21, T, ink);
        c.H(x0 + 5, x0 + 29, 88, ink);
        // P
        x0 = 418;
        c.V(x0, T - 6, B + 6, ink);
        c.H(x0, x0 + W, T, ink);
        c.V(x0 + W, T - 6, M + 6, ink);
        c.H(x0, x0 + W, M, ink);
        // E
        x0 = 475;
        c.V(x0, T - 6, B + 6, ink);
        c.H(x0, x0 + W, T, ink);
        c.H(x0, x0 + W, B, ink);
        c.H(x0, x0 + 28, M, ink);
        // R
        x0 = 532;
        c.V(x0, T - 6, B + 6, ink);
        c.H(x0, x0 + W, T, ink);
        c.V(x0 + W, T - 6, M + 6, ink);
        c.H(x0, x0 + W, M, ink);
        c.line(x0 + 14, M + 2, x0 + W, B + 2, ink);
        // I
        c.V(589, T - 6, B + 6, ink);
        // O (room + core)
        x0 = 612;
        c.V(x0, T - 6, B + 6, ink);
        c.V(x0 + W, T - 6, B + 6, ink);
        c.H(x0, x0 + W, T, ink);
        c.H(x0, x0 + W, B, ink);
        c.rect(623, 74, 635, 86, c.core);
        // M
        x0 = 669;
        c.V(x0, T - 6, B + 6, ink);
        c.V(x0 + 44, T - 6, B + 6, ink);
        c.line(x0, T, x0 + 22, 82, ink);
        c.line(x0 + 22, 82, x0 + 44, T, ink);
    }

    static void draw(lv_event_t* e) {
        auto* obj = lv_event_get_target_obj(e);
        const intptr_t flags = (intptr_t)lv_event_get_user_data(e);
        const bool lockup = flags & MODE_LOCKUP;
        const bool dark = flags & FLAG_DARK;

        Ctx c;
        c.layer = lv_event_get_layer(e);
        lv_obj_get_coords(obj, &c.b);
        c.w = lv_obj_get_width(obj);
        c.designW = lockup ? 760 : 120;
        c.ox = lockup ? 40 : 0;
        c.oy = lockup ? 20 : 0;

        const auto& palette = UITheme::palette();
        c.mark = dark ? palette.accent : palette.strong;
        c.core = dark ? palette.strong : palette.accent;
        c.ink = dark ? 0xF8FAFC : palette.ink;
        emblem(c);
        if (lockup)
            wordmark(c, c.ink);
    }
};