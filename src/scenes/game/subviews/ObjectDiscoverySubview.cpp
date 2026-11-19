#include "ObjectDiscoverySubview.h"
#include "systems/ui/UITheme.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/ImageView.h"
#include "systems/ui/localization/I18n.h"
#include <algorithm>

void ObjectDiscoverySubview::create(lv_obj_t* parent) {
    root = lv_obj_create(parent);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(root, static_cast<lv_obj_flag_t>(LV_OBJ_FLAG_SCROLLABLE |
                                                       LV_OBJ_FLAG_GESTURE_BUBBLE));
    lv_obj_add_flag(root, static_cast<lv_obj_flag_t>(LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_HIDDEN |
                                                     LV_OBJ_FLAG_FLOATING));
    lv_obj_align(root, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x101C24), 0);
    lv_obj_set_style_bg_opa(root, 190, 0);
    item = lv_obj_create(root);
    lv_obj_remove_style_all(item);
    lv_obj_set_size(item, 112, 112);
    lv_obj_clear_flag(item,
                      static_cast<lv_obj_flag_t>(LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE));
    lv_obj_set_style_radius(item, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(item, 3, 0);
    lv_obj_set_style_shadow_width(item, 16, 0);
    lv_obj_set_style_shadow_opa(item, 100, 0);
    lv_obj_set_style_bg_opa(item, LV_OPA_COVER, 0);
    lv_obj_set_style_transform_pivot_x(item, 56, 0);
    lv_obj_set_style_transform_pivot_y(item, 56, 0);
    caption = Label::create(root, "", UITheme::heading(18), LV_TEXT_ALIGN_CENTER, LV_PCT(100));
    lv_obj_align(caption, LV_ALIGN_TOP_MID, 0, 42);
    lv_obj_set_style_text_color(caption, lv_color_hex(0xF7FAFC), 0);
    title = Label::create(root, "", &lv_font_montserrat_16, LV_TEXT_ALIGN_CENTER, LV_PCT(90));
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_height(title, 22);
    lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF7FAFC), 0);
}

void ObjectDiscoverySubview::translate() {
    if (caption)
        Label::setText(caption, tr(isHint ? Text::HintUnlocked : Text::ObjectDiscovered));
}

void ObjectDiscoverySubview::show(const GameObjectSubview::ObjectData& object,
                                  lv_obj_t* destination, uint32_t now, bool hint) {
    isHint = hint;
    if (!root || !destination)
        return;
    lv_obj_update_layout(lv_obj_get_parent(root));
    lv_area_t area, origin;
    lv_obj_get_coords(destination, &area);
    lv_obj_get_coords(root, &origin);
    startX = lv_obj_get_width(root) / 2;
    startY = lv_obj_get_height(root) / 2 + 10;
    targetX = (area.x1 + area.x2) / 2 - origin.x1;
    targetY = (area.y1 + area.y2) / 2 - origin.y1;
    lv_obj_clean(item);
    lv_obj_set_style_bg_color(item, lv_color_hex(UITheme::palette().soft), 0);
    lv_obj_set_style_border_color(item, lv_color_hex(UITheme::palette().accent), 0);
    lv_obj_set_style_shadow_color(item, lv_color_hex(UITheme::palette().accent), 0);
    std::string path = object.image;
    if (path.empty())
        for (const auto& block : object.content)
            if (block.type == GameObjectSubview::ContentType::Image && !block.imagePath.empty()) {
                path = block.imagePath;
                break;
            }
    lv_image_header_t info{};
    const auto source = normalizeImageViewSrc(path.c_str());
    if (!source.empty() && lv_image_decoder_get_info(source.c_str(), &info) == LV_RESULT_OK &&
        info.w && info.h) {
        auto* image = ImageView::create(item, path.c_str());
        lv_obj_set_size(image, 84, 84);
        lv_image_set_inner_align(image, LV_IMAGE_ALIGN_CONTAIN);
        lv_obj_center(image);
    } else {
        auto* icon = Label::create(item, hint ? "?" : LV_SYMBOL_FILE, &lv_font_montserrat_32);
        UITheme::accent(icon);
        lv_obj_center(icon);
    }
    translate();
    Label::setText(title, object.title.c_str());
    lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(root);
    started = now;
    rendered = now - 34;
    active = true;
    update(now);
}

bool ObjectDiscoverySubview::update(uint32_t now) {
    if (!active)
        return false;
    const uint32_t elapsed = now - started;
    if (elapsed >= DurationMs) {
        hide();
        return true;
    }
    if (uint32_t(now - rendered) < 33)
        return false;
    rendered = now;
    int32_t x = startX, y = startY, scale = 256;
    if (elapsed < RevealMs) {
        const auto t = std::min<uint32_t>(elapsed, 240);
        scale = 192 + 64 * t / 240;
    } else {
        const int32_t t = (elapsed - RevealMs) * 1024 / FlightMs;
        const int32_t eased = t * t * (3072 - 2 * t) / (1024 * 1024);
        x += (targetX - startX) * eased / 1024;
        y += (targetY - startY) * eased / 1024 - 56 * t * (1024 - t) / (1024 * 1024);
        scale = 256 - 192 * eased / 1024;
    }
    lv_obj_set_pos(item, x - 56, y - 56);
    lv_obj_set_style_transform_scale_x(item, scale, 0);
    lv_obj_set_style_transform_scale_y(item, scale, 0);
    const auto textOpacity =
        elapsed < RevealMs ? LV_OPA_COVER : 255 - 255 * (elapsed - RevealMs) / FlightMs;
    lv_obj_set_style_opa(caption, textOpacity, 0);
    lv_obj_set_style_opa(title, textOpacity, 0);
    return false;
}

void ObjectDiscoverySubview::hide() {
    active = false;
    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void ObjectDiscoverySubview::destroy() {
    hide();
    if (root)
        lv_obj_delete(root);
    root = item = caption = title = nullptr;
}
