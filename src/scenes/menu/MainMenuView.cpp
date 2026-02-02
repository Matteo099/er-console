#include "MainMenuView.h"
#include "log/Logger.h"
#include <cstring>

#include "systems/ui/components/BrandHeader.h"

void MainMenuView::create()
{
    Log::debug(Log::UI_TAG, "Creating MainMenuView");
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // ---- Brand image (top) ----
    brandImg = BrandHeader::create(screen);

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

    // --- Button factory ---
    auto createButton = [&](const char *label, const char *action)
    {
        lv_obj_t *btn = lv_btn_create(menuContainer);
        lv_obj_set_width(btn, 200);
        lv_obj_set_height(btn, 42);
        lv_obj_add_event_cb(
            btn,
            onButtonClicked,
            LV_EVENT_CLICKED,
            (void *)action);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, label);
        lv_obj_set_style_text_font(lbl, &lv_font_montserrat_16, 0);
        lv_obj_center(lbl);

        return btn;
    };

    // --- Buttons ---
    btnNewGame = createButton("New Game", "new_game");
    btnSettings = createButton("Settings", "settings");
    btnAbout = createButton("About", "about");
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
    btnNewGame = btnSettings = btnAbout = nullptr;
}

void MainMenuView::onButtonClicked(lv_event_t *e)
{
    auto *view =
        static_cast<MainMenuView *>(lv_event_get_user_data(e));
    const char *action =
        static_cast<const char *>(lv_event_get_user_data(e));

    if (!view || !action)
        return;

    if (strcmp(action, "new_game") == 0)
        view->pendingAction = MainMenuAction::NewGame;
    else if (strcmp(action, "settings") == 0)
        view->pendingAction = MainMenuAction::Settings;
    else if (strcmp(action, "about") == 0)
        view->pendingAction = MainMenuAction::About;
}

MainMenuAction MainMenuView::consumeAction()
{
    MainMenuAction a = pendingAction;
    pendingAction = MainMenuAction::None;
    return a;
}
