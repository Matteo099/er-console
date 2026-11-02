#include "UITheme.h"
#include <cstring>
#include <initializer_list>
// LVGL 9.4 keeps its theme descriptor here; contained in this adapter.
#include <src/themes/lv_theme_private.h>

namespace
{
    const UITheme::Palette palettes[] = {
        {0xF3F8F5, 0xFFFFFF, 0xE8F2EC, 0xC8E7D8, 0x2D6652, 0x263D34, 0x5D7064, 0xDCE8E0},
        {0xF3F7FC, 0xFFFFFF, 0xEAF1FB, 0xD3E5FA, 0x375F89, 0x293B52, 0x5B6F87, 0xDDE6F1},
        {0xFFF6F3, 0xFFFFFF, 0xFBECE7, 0xF5D5CC, 0x99554B, 0x4B3532, 0x805F57, 0xEEDFD9},
    };
    UIThemeId active = UIThemeId::Mint;
    lv_display_t *display = nullptr;
    lv_theme_t theme{};
    lv_style_t pageStyle, cardStyle, softStyle, textStyle, mutedStyle, accentStyle;
    lv_style_t buttonStyle, pressedStyle, secondaryStyle, checkedStyle, disabledStyle;
    lv_style_t trackStyle, indicatorStyle, dangerStyle;
    bool initialized = false;

    void applyTheme(lv_theme_t *, lv_obj_t *obj)
    {
        if (!lv_obj_get_parent(obj)) lv_obj_add_style(obj, &pageStyle, 0);
        if (lv_obj_check_type(obj, &lv_label_class)) lv_obj_add_style(obj, &textStyle, 0);
        if (lv_obj_check_type(obj, &lv_button_class))
        {
            lv_obj_add_style(obj, &buttonStyle, 0);
            lv_obj_add_style(obj, &pressedStyle, LV_STATE_PRESSED);
            lv_obj_add_style(obj, &checkedStyle, LV_STATE_CHECKED);
            lv_obj_add_style(obj, &disabledStyle, LV_STATE_DISABLED);
        }
        if (lv_obj_check_type(obj, &lv_bar_class) || lv_obj_check_type(obj, &lv_slider_class))
        {
            lv_obj_add_style(obj, &trackStyle, LV_PART_MAIN);
            lv_obj_add_style(obj, &indicatorStyle, LV_PART_INDICATOR);
        }
        if (lv_obj_check_type(obj, &lv_arc_class))
        {
            lv_obj_set_style_arc_width(obj, 7, LV_PART_MAIN);
            lv_obj_set_style_arc_width(obj, 7, LV_PART_INDICATOR);
            lv_obj_add_style(obj, &indicatorStyle, LV_PART_INDICATOR);
        }
    }
    void updateStyles()
    {
        const auto &p = UITheme::palette();
        lv_style_set_bg_color(&pageStyle, lv_color_hex(p.background));
        lv_style_set_text_color(&pageStyle, lv_color_hex(p.ink));
        lv_style_set_text_font(&pageStyle, UITheme::font(&lv_font_montserrat_14));
        lv_style_set_text_color(&textStyle, lv_color_hex(p.ink));
        lv_style_set_text_font(&textStyle, UITheme::font(&lv_font_montserrat_14));
        lv_style_set_bg_color(&cardStyle, lv_color_hex(p.surface));
        lv_style_set_border_color(&cardStyle, lv_color_hex(p.border));
        lv_style_set_bg_color(&softStyle, lv_color_hex(p.soft));
        lv_style_set_text_color(&mutedStyle, lv_color_hex(p.muted));
        lv_style_set_text_color(&accentStyle, lv_color_hex(p.strong));
        lv_style_set_bg_color(&buttonStyle, lv_color_hex(p.accent));
        lv_style_set_text_color(&buttonStyle, lv_color_hex(p.ink));
        lv_style_set_bg_color(&pressedStyle, lv_color_hex(p.soft));
        lv_style_set_bg_color(&secondaryStyle, lv_color_hex(p.surface));
        lv_style_set_border_color(&secondaryStyle, lv_color_hex(p.border));
        lv_style_set_border_color(&checkedStyle, lv_color_hex(p.strong));
        lv_style_set_bg_color(&checkedStyle, lv_color_hex(p.accent));
        lv_style_set_bg_color(&trackStyle, lv_color_hex(p.soft));
        lv_style_set_bg_color(&indicatorStyle, lv_color_hex(p.strong));
        lv_style_set_arc_color(&indicatorStyle, lv_color_hex(p.strong));
    }
}

void UITheme::init(lv_display_t *disp)
{
    if (initialized) return;
    display = disp;
    for (auto *style : {&pageStyle, &cardStyle, &softStyle, &textStyle, &mutedStyle,
                       &accentStyle, &buttonStyle, &pressedStyle, &secondaryStyle,
                       &checkedStyle, &disabledStyle, &trackStyle, &indicatorStyle, &dangerStyle})
        lv_style_init(style);
    lv_style_set_bg_opa(&pageStyle, LV_OPA_COVER);
    lv_style_set_bg_opa(&cardStyle, LV_OPA_COVER);
    lv_style_set_radius(&cardStyle, 14);
    lv_style_set_border_width(&cardStyle, 1);
    lv_style_set_pad_all(&cardStyle, 12);
    lv_style_set_bg_opa(&softStyle, LV_OPA_COVER);
    lv_style_set_bg_opa(&buttonStyle, LV_OPA_COVER);
    lv_style_set_radius(&buttonStyle, 12);
    lv_style_set_border_width(&buttonStyle, 0);
    lv_style_set_shadow_width(&buttonStyle, 0);
    lv_style_set_outline_width(&buttonStyle, 0);
    lv_style_set_pad_all(&buttonStyle, 8);
    lv_style_set_border_width(&secondaryStyle, 1);
    lv_style_set_border_width(&checkedStyle, 2);
    lv_style_set_opa(&disabledStyle, LV_OPA_40);
    lv_style_set_radius(&trackStyle, LV_RADIUS_CIRCLE);
    lv_style_set_radius(&indicatorStyle, LV_RADIUS_CIRCLE);
    lv_style_set_text_color(&dangerStyle, lv_color_hex(0xA44848));
    updateStyles();
    theme.disp = display;
    theme.color_primary = lv_color_hex(palette().strong);
    theme.color_secondary = theme.color_primary;
    theme.font_small = font(&lv_font_montserrat_12);
    theme.font_normal = font(&lv_font_montserrat_14);
    theme.font_large = font(&lv_font_montserrat_20);
    lv_theme_set_parent(&theme, lv_theme_default_init(display, lv_color_hex(palette().strong),
                                                   lv_color_hex(palette().strong), false, font(&lv_font_montserrat_14)));
    lv_theme_set_apply_cb(&theme, applyTheme);
    lv_display_set_theme(display, &theme);
    initialized = true;
}
void UITheme::select(UIThemeId id)
{
    if (static_cast<unsigned>(id) > 2) id = UIThemeId::Mint;
    active = id;
    if (!initialized) return;
    updateStyles();
    theme.color_primary = lv_color_hex(palette().strong);
    theme.color_secondary = theme.color_primary;
    lv_theme_default_init(display, lv_color_hex(palette().strong), lv_color_hex(palette().strong),
                          false, font(&lv_font_montserrat_14));
    lv_display_set_theme(display, &theme);
    lv_obj_report_style_change(nullptr);
}
UIThemeId UITheme::current() { return active; }
const UITheme::Palette &UITheme::palette() { return palette(active); }
const UITheme::Palette &UITheme::palette(UIThemeId id) { return palettes[static_cast<unsigned>(id) % 3]; }
UIThemeId UITheme::fromName(const char *value)
{
    if (value && (!std::strcmp(value, "sky") || !std::strcmp(value, "blue"))) return UIThemeId::Sky;
    if (value && (!std::strcmp(value, "coral") || !std::strcmp(value, "red"))) return UIThemeId::Coral;
    return UIThemeId::Mint;
}
const char *UITheme::name(UIThemeId id) { return id == UIThemeId::Sky ? "sky" : id == UIThemeId::Coral ? "coral" : "mint"; }
const char *UITheme::title(UIThemeId id) { return id == UIThemeId::Sky ? "Cielo" : id == UIThemeId::Coral ? "Corallo" : "Menta"; }
const lv_font_t *UITheme::font(const lv_font_t *requested)
{
    // Built-in Montserrat omits Latin-1 accents; reuse the existing project font
    // as a fallback, without allocating bitmaps or loading fonts from the SD.
    static lv_font_t fonts[] = {lv_font_montserrat_12, lv_font_montserrat_14, lv_font_montserrat_16,
        lv_font_montserrat_18, lv_font_montserrat_20, lv_font_montserrat_24, lv_font_montserrat_28};
    static const lv_font_t *base[] = {&lv_font_montserrat_12, &lv_font_montserrat_14, &lv_font_montserrat_16,
        &lv_font_montserrat_18, &lv_font_montserrat_20, &lv_font_montserrat_24, &lv_font_montserrat_28};
    static const lv_font_t *fallback[] = {&lv_font_roboto_mono_12, &lv_font_roboto_mono_14,
        &lv_font_roboto_mono_16, &lv_font_roboto_mono_18, &lv_font_roboto_mono_20,
        &lv_font_roboto_mono_28, &lv_font_roboto_mono_28};
    for (unsigned i = 0; i < 7; ++i)
    {
        fonts[i].fallback = fallback[i];
        if (requested == base[i] || (i < 5 && requested == fallback[i])) return &fonts[i];
    }
    if (requested == &lv_font_roboto_mono_28) return &fonts[5];
    if (requested == &lv_font_roboto_mono_30) return &fonts[6];
    return requested ? requested : &fonts[1];
}
void UITheme::screen(lv_obj_t *obj) { lv_obj_add_style(obj, &pageStyle, 0); }
void UITheme::card(lv_obj_t *obj)
{
    // Container's transparent local defaults would otherwise override shared styles.
    const lv_style_prop_t props[] = {LV_STYLE_BG_OPA, LV_STYLE_BG_COLOR, LV_STYLE_BORDER_WIDTH,
        LV_STYLE_PAD_TOP, LV_STYLE_PAD_BOTTOM, LV_STYLE_PAD_LEFT, LV_STYLE_PAD_RIGHT};
    for (auto prop : props) lv_obj_remove_local_style_prop(obj, prop, 0);
    lv_obj_add_style(obj, &cardStyle, 0);
}
void UITheme::soft(lv_obj_t *obj) { lv_obj_add_style(obj, &softStyle, 0); }
void UITheme::muted(lv_obj_t *obj) { lv_obj_add_style(obj, &mutedStyle, 0); }
void UITheme::accent(lv_obj_t *obj) { lv_obj_add_style(obj, &accentStyle, 0); }
void UITheme::secondary(lv_obj_t *obj) { lv_obj_add_style(obj, &secondaryStyle, 0); }
void UITheme::danger(lv_obj_t *obj) { lv_obj_add_style(obj, &dangerStyle, 0); }
