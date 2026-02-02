#include "GameView.h"

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
    /* Root */
    root = lv_obj_create(nullptr);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    /* ================= HEADER ================= */
    header = lv_obj_create(root);
    lv_obj_set_size(header, LV_PCT(100), 48);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, 0);

    /* Game title (left) */
    titleLabel = lv_label_create(header);
    lv_label_set_text(titleLabel, "Escape Room");
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_16, 0);
    lv_obj_align(titleLabel, LV_ALIGN_LEFT_MID, 12, 0);

    /* Info button (right) */
    infoBtn = lv_btn_create(header);
    lv_obj_set_size(infoBtn, 36, 36);
    lv_obj_align(infoBtn, LV_ALIGN_RIGHT_MID, -52, 0);
    lv_obj_add_event_cb(infoBtn, onInfo, LV_EVENT_CLICKED, this);

    auto* infoLbl = lv_label_create(infoBtn);
    lv_label_set_text(infoLbl, "i");
    lv_obj_center(infoLbl);

    /* Settings button (rightmost) */
    settingsBtn = lv_btn_create(header);
    lv_obj_set_size(settingsBtn, 36, 36);
    lv_obj_align(settingsBtn, LV_ALIGN_RIGHT_MID, -8, 0);
    lv_obj_add_event_cb(settingsBtn, onSettings, LV_EVENT_CLICKED, this);

    auto* setLbl = lv_label_create(settingsBtn);
    lv_label_set_text(setLbl, "⚙");
    lv_obj_center(setLbl);

    /* ================= TIMER ================= */
    timerLabel = lv_label_create(root);
    lv_obj_set_style_text_font(timerLabel, &lv_font_montserrat_48, 0);
    lv_label_set_text(timerLabel, "01:00:00");
    lv_obj_align(timerLabel, LV_ALIGN_CENTER, 0, -20);

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
