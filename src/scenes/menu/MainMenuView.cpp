#include "MainMenuView.h"
#include <cstring>


void MainMenuView::create()
{
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // --- Brand ---
    brandImg = lv_img_create(screen);
    lv_img_set_src(brandImg, "S:/brand.png");
    lv_obj_align(brandImg, LV_ALIGN_TOP_MID, 0, 12);

    // --- Menu container ---
    menuContainer = lv_obj_create(screen);
    lv_obj_set_size(menuContainer, 220, 140);
    lv_obj_align(menuContainer, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_flex_flow(menuContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menuContainer,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(menuContainer, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(menuContainer, LV_OBJ_FLAG_SCROLLABLE);

    auto createButton = [&](const char* text) {
        lv_obj_t* btn = lv_btn_create(menuContainer);
        lv_obj_set_width(btn, 200);
        lv_obj_add_event_cb(btn, onButtonClicked, LV_EVENT_CLICKED, (void*)text);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, text);
        lv_obj_center(lbl);
        return btn;
    };

    btnStart    = createButton("Start");
    btnSettings = createButton("Settings");
    btnAbout    = createButton("About");
}

void MainMenuView::update()
{
    // Nothing animated yet (future: focus animations, idle effects)
}

void MainMenuView::destroy()
{
    if (screen)
        lv_obj_del(screen);

    screen = nullptr;
    brandImg = nullptr;
    menuContainer = nullptr;
    btnStart = btnSettings = btnAbout = nullptr;
}

void MainMenuView::onButtonClicked(lv_event_t* e)
{
    const char* action = static_cast<const char*>(lv_event_get_user_data(e));

    if (!action) return;

    if (strcmp(action, "Start") == 0) {
        // TODO: change to game scene
    }
    else if (strcmp(action, "Settings") == 0) {
        // TODO
    }
    else if (strcmp(action, "About") == 0) {
        // TODO
    }
}
