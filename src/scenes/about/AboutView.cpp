#include "AboutView.h"

void AboutView::create()
{
    // --- Screen ---
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // --- Brand ---
    brandImg = lv_img_create(screen);
    lv_img_set_src(brandImg, "S:/brand.png");
    lv_obj_align(brandImg, LV_ALIGN_TOP_MID, 0, 12);

    // --- Content container ---
    container = lv_obj_create(screen);
    lv_obj_set_size(container, 220, 120);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(
        container,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    // --- About text ---
    textLabel = lv_label_create(container);
    lv_label_set_text(
        textLabel,
        "EscapeOS\n\n"
        "Open escape-room engine\n"
        "for ESP32-based consoles.\n\n"
        "© 2026"
    );

    lv_obj_set_style_text_align(textLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(textLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(textLabel, lv_color_hex(0xCCCCCC), 0);

    // --- Back button ---
    btnBack = lv_btn_create(screen);
    lv_obj_set_size(btnBack, 90, 36);
    lv_obj_align(btnBack, LV_ALIGN_BOTTOM_MID, 0, -12);
    lv_obj_add_event_cb(
        btnBack,
        onBackClicked,
        LV_EVENT_CLICKED,
        this);

    lv_obj_t* lbl = lv_label_create(btnBack);
    lv_label_set_text(lbl, "Back");
    lv_obj_center(lbl);
}

void AboutView::update()
{
    // Static screen — nothing animated for now
}

void AboutView::destroy()
{
    if (screen)
        lv_obj_del(screen);

    screen = nullptr;
    brandImg = nullptr;
    container = nullptr;
    textLabel = nullptr;
    btnBack = nullptr;

    pendingAction = AboutAction::None;
}

void AboutView::onBackClicked(lv_event_t* e)
{
    auto* view =
        static_cast<AboutView*>(lv_event_get_user_data(e));

    if (!view)
        return;

    view->pendingAction = AboutAction::Back;
}

AboutAction AboutView::consumeAction()
{
    AboutAction a = pendingAction;
    pendingAction = AboutAction::None;
    return a;
}
