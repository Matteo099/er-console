#pragma once
#include <lvgl.h>
#include "systems/ui/UITheme.h"

// Flat firefly mark, drawn locally so headers never depend on SD assets.
class LimanulabLogo
{
public:
    static lv_obj_t *create(lv_obj_t *parent)
    {
        auto *mark = lv_obj_create(parent);
        lv_obj_remove_style_all(mark);
        lv_obj_set_size(mark, 32, 32);
        lv_obj_clear_flag(mark, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(mark, LV_OBJ_FLAG_SCROLLABLE);
        UITheme::soft(mark);
        UITheme::accent(mark);
        lv_obj_set_style_radius(mark, 10, 0);
        lv_obj_add_event_cb(mark, draw, LV_EVENT_DRAW_MAIN, nullptr);
        return mark;
    }

private:
    static void draw(lv_event_t *event)
    {
        auto *obj = lv_event_get_target_obj(event);
        auto *layer = lv_event_get_layer(event);
        lv_area_t bounds;
        lv_obj_get_coords(obj, &bounds);
        const auto ink = lv_obj_get_style_text_color(obj, LV_PART_MAIN);
        const auto glow = lv_obj_get_style_bg_color(obj, LV_PART_MAIN);
        auto point = [&](int x, int y) -> lv_point_precise_t { return {bounds.x1 + x, bounds.y1 + y}; };
        auto stroke = [&](int x1, int y1, int x2, int y2, int width, lv_color_t color)
        {
            lv_draw_line_dsc_t line;
            lv_draw_line_dsc_init(&line);
            line.p1 = point(x1, y1); line.p2 = point(x2, y2);
            line.width = width; line.color = color;
            line.round_start = line.round_end = 1;
            lv_draw_line(layer, &line);
        };
        auto capsule = [&](int x1, int y1, int x2, int y2, lv_color_t color)
        {
            lv_draw_rect_dsc_t rect;
            lv_draw_rect_dsc_init(&rect);
            rect.bg_color = color; rect.bg_opa = LV_OPA_COVER;
            rect.radius = LV_RADIUS_CIRCLE;
            lv_area_t area = {bounds.x1 + x1, bounds.y1 + y1, bounds.x1 + x2, bounds.y1 + y2};
            lv_draw_rect(layer, &rect, &area);
        };
        stroke(13, 10, 10, 5, 2, ink);
        stroke(19, 10, 22, 5, 2, ink);
        stroke(11, 16, 5, 22, 4, ink);
        stroke(21, 16, 27, 22, 4, ink);
        stroke(12, 20, 8, 26, 3, ink);
        stroke(20, 20, 24, 26, 3, ink);
        capsule(11, 15, 21, 28, ink);
        capsule(14, 20, 18, 25, glow);
        capsule(10, 9, 22, 20, ink);
        capsule(13, 12, 19, 17, glow);
    }
};
