#include "GameView.h"

#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/BackButton.h"
#include "Arduino.h"

void GameView::create()
{
    /* Root */
    root = lv_obj_create(nullptr);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // --- Brand ---
    brandImg = BrandHeader::create(root);

    // --- Content container ---
    container = lv_obj_create(root);
    lv_obj_set_size(container, 220, 120);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    // --- Game text ---
    textLabel = lv_label_create(container);
    lv_label_set_text(textLabel,
        "EscapeOS\n\n"
        "Open escape-room engine\n"
        "for ESP32-based consoles.\n\n"
        "© 2026");

    lv_obj_set_style_text_align(textLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(textLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(textLabel, lv_color_hex(0xCCCCCC), 0);

    /* ================= TIMER ================= */
    timerLabel = lv_label_create(root);
    lv_obj_set_style_text_font(timerLabel, &lv_font_montserrat_48, 0);
    lv_label_set_text(timerLabel, "00:00:00");
    lv_obj_align(timerLabel, LV_ALIGN_CENTER, 0, -40);

    /* ================= START / PAUSE ================= */
    startPauseBtn = lv_btn_create(root);
    lv_obj_set_size(startPauseBtn, 120, 44);
    lv_obj_align(startPauseBtn, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_obj_add_event_cb(startPauseBtn, onStartPause, LV_EVENT_CLICKED, this);

    startPauseLabel = lv_label_create(startPauseBtn);
    lv_label_set_text(startPauseLabel, "Start");
    lv_obj_center(startPauseLabel);

    /* ================= CANCEL ================= */
    cancelBtn = lv_btn_create(root);
    lv_obj_set_size(cancelBtn, 120, 44);
    lv_obj_align(cancelBtn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    lv_obj_add_event_cb(cancelBtn, onCancel, LV_EVENT_CLICKED, this);

    auto* cancelLbl = lv_label_create(cancelBtn);
    lv_label_set_text(cancelLbl, "Cancel");
    lv_obj_center(cancelLbl);
}

void GameView::update()
{
    /* no periodic UI logic */
}

void GameView::destroy()
{
    if (root)
        lv_obj_del(root);

    root = nullptr;
    header = nullptr;
    titleLabel = nullptr;
    timerLabel = nullptr;
    startPauseBtn = nullptr;
    startPauseLabel = nullptr;
    cancelBtn = nullptr;
    infoBtn = nullptr;
    settingsBtn = nullptr;
}

/* ================= API ================= */

void GameView::setGameTitle(const std::string& title)
{
    lv_label_set_text(titleLabel, title.c_str());
}

void GameView::setTime(uint32_t seconds)
{
    lv_label_set_text(timerLabel, String(seconds).c_str());
}

void GameView::setRunning(bool running)
{
    lv_label_set_text(startPauseLabel, running ? "Pause" : "Start");
}

/* ================= EVENTS ================= */

void GameView::onStartPause(lv_event_t* e)
{
    auto* view = static_cast<GameView*>(lv_event_get_user_data(e));
    view->startPressed = true;
}

void GameView::onCancel(lv_event_t* e)
{
    auto* view = static_cast<GameView*>(lv_event_get_user_data(e));
    view->cancelPressed = true;
}

void GameView::onInfo(lv_event_t* e)
{
    auto* view = static_cast<GameView*>(lv_event_get_user_data(e));
    view->infoPressed = true;
}

void GameView::onSettings(lv_event_t* e)
{
    auto* view = static_cast<GameView*>(lv_event_get_user_data(e));
    view->settingsPressed = true;
}
