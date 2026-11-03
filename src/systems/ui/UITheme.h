#pragma once
#include <lvgl.h>

enum class UIThemeId : uint8_t { Mint, Sky, Coral };
enum class UIStatusTone : uint8_t { Good, Warning, Error };

// Views compose semantic styles; scenes choose and persist preferences.
class UITheme
{
public:
    struct Palette { uint32_t background, surface, soft, accent, strong, ink, muted, border; };
    static void init(lv_display_t *display);
    static void select(UIThemeId id);
    static UIThemeId current();
    static UIThemeId fromName(const char *name);
    static const char *name(UIThemeId id);
    static const char *title(UIThemeId id);
    static const Palette &palette(UIThemeId id);
    static const Palette &palette();
    static lv_color_t statusColor(UIStatusTone tone);
    static const lv_font_t *font(const lv_font_t *requested);
    static void screen(lv_obj_t *obj);
    static void card(lv_obj_t *obj);
    static void soft(lv_obj_t *obj);
    static void muted(lv_obj_t *obj);
    static void accent(lv_obj_t *obj);
    static void secondary(lv_obj_t *obj);
    static void danger(lv_obj_t *obj);
};
