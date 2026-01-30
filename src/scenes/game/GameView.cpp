#include "GameView.h"

#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/BackButton.h"

void GameView::create()
{
    // --- Screen ---
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // --- Brand ---
    brandImg = BrandHeader::create(screen);

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

    // --- Game text ---
    textLabel = lv_label_create(container);
    lv_label_set_text(
        textLabel,
        "EscapeOS\n\n"
        "Open escape-room engine\n"
        "for ESP32-based consoles.\n\n"
        "© 2026");

    lv_obj_set_style_text_align(textLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(textLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(textLabel, lv_color_hex(0xCCCCCC), 0);

    // --- Back button ---
    btnBack = BackButton::create(
        screen,
        onBackClicked,
        this);
}

void GameView::update()
{
    // Static screen — nothing animated for now
}

void GameView::destroy()
{
    if (screen)
        lv_obj_del(screen);

    screen = nullptr;
    brandImg = nullptr;
    container = nullptr;
    textLabel = nullptr;
    btnBack = nullptr;

    pendingAction = GameAction::None;
}

void GameView::onBackClicked(lv_event_t *e)
{
    auto *view =
        static_cast<GameView *>(lv_event_get_user_data(e));

    if (!view)
        return;

    view->pendingAction = GameAction::Back;
}

GameAction GameView::consumeAction()
{
    GameAction a = pendingAction;
    pendingAction = GameAction::None;
    return a;
}
