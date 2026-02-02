#include "GameView.h"
#include "log/Logger.h"

static std::string formatTime(uint32_t sec)
{
    uint32_t h = sec / 3600;
    uint32_t m = (sec % 3600) / 60;
    uint32_t s = sec % 60;

    char buf[16];
    snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu", h, m, s);
    return buf;
}

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
    lv_label_set_text(timerLabel, formatTime(seconds).c_str());
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
