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

    /* ================= GAME CONTENT AREA ================= */
    gameContentArea = lv_obj_create(root);
    lv_obj_set_size(gameContentArea, LV_PCT(100), LV_PCT(100) - 96 - 48);
    lv_obj_set_pos(gameContentArea, 0, 48);
    lv_obj_clear_flag(gameContentArea, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(gameContentArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(gameContentArea, LV_OPA_COVER, 0);

    /* ================= TIMER ================= */
    timerLabel = lv_label_create(root);
    lv_obj_set_style_text_font(timerLabel, &lv_font_montserrat_48, 0);
    lv_label_set_text(timerLabel, "01:00:00");
    lv_obj_align(timerLabel, LV_ALIGN_CENTER, 0, -20);

    /* ================= LOADING BAR ================= */
    loadingContainer = lv_obj_create(root);
    lv_obj_set_size(loadingContainer, 200, 100);
    lv_obj_align(loadingContainer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(loadingContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(loadingContainer, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(loadingContainer, LV_OPA_COVER, 0);

    loadingLabel = lv_label_create(loadingContainer);
    lv_label_set_text(loadingLabel, "Loading game...");
    lv_obj_align(loadingLabel, LV_ALIGN_TOP_MID, 0, 10);

    loadingBar = lv_bar_create(loadingContainer);
    lv_obj_set_size(loadingBar, 160, 20);
    lv_bar_set_value(loadingBar, 50, LV_ANIM_OFF);
    lv_obj_align(loadingBar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align_to(loadingBar, loadingLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // Hide loading initially
    lv_obj_add_flag(loadingContainer, LV_OBJ_FLAG_HIDDEN);

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
    loadingContainer = nullptr;
    loadingBar = nullptr;
    loadingLabel = nullptr;
    errorLabel = nullptr;
    gameContentArea = nullptr;
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

void GameView::showLoadingBar()
{
    if (loadingContainer)
        lv_obj_clear_flag(loadingContainer, LV_OBJ_FLAG_HIDDEN);
}

void GameView::hideLoadingBar()
{
    if (loadingContainer)
        lv_obj_add_flag(loadingContainer, LV_OBJ_FLAG_HIDDEN);
}

void GameView::setErrorMessage(const std::string& message)
{
    hideLoadingBar();

    if (!errorLabel)
    {
        errorLabel = lv_label_create(root);
        lv_obj_set_size(errorLabel, LV_PCT(80), LV_SIZE_CONTENT);
        lv_obj_align(errorLabel, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_long_mode(errorLabel, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_color(errorLabel, lv_color_hex(0xFF0000), 0);
    }

    lv_label_set_text(errorLabel, message.c_str());
}

/* ================= LUA API - Display Control ================= */

void GameView::clearDisplay()
{
    if (gameContentArea)
    {
        lv_obj_clean(gameContentArea);
    }
}

void GameView::drawText(int x, int y, const char* text, int fontSize, uint32_t color)
{
    if (!gameContentArea || !text)
        return;

    lv_obj_t* label = lv_label_create(gameContentArea);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);

    // Select font based on size
    const lv_font_t* font = &lv_font_montserrat_14;
    if (fontSize >= 24)
        font = &lv_font_montserrat_28;
    else if (fontSize >= 20)
        font = &lv_font_montserrat_20;
    else if (fontSize >= 16)
        font = &lv_font_montserrat_16;

    lv_obj_set_style_text_font(label, font, 0);
}

void GameView::drawRect(int x, int y, int width, int height, uint32_t color, bool fill)
{
    if (!gameContentArea)
        return;

    lv_obj_t* rect = lv_obj_create(gameContentArea);
    lv_obj_set_size(rect, width, height);
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_style_bg_color(rect, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(rect, fill ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_color(rect, lv_color_hex(color), 0);
    lv_obj_set_style_border_width(rect, fill ? 0 : 2, 0);
}

void GameView::drawLine(int x1, int y1, int x2, int y2, uint32_t color)
{
    if (!gameContentArea)
        return;

    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = lv_color_hex(color);
    line_dsc.width = 2;

    // Note: Direct line drawing requires canvas or other approach
    // For now, we use a thin rectangle as approximation
    drawRect(x1, y1, x2 - x1, 2, color, true);
}

/* ================= LUA API - UI Components ================= */

lv_obj_t* GameView::createButton(int x, int y, int width, int height, const char* text)
{
    if (!gameContentArea)
        return nullptr;

    lv_obj_t* btn = lv_btn_create(gameContentArea);
    lv_obj_set_size(btn, width, height);
    lv_obj_set_pos(btn, x, y);

    if (text)
    {
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, text);
        lv_obj_center(label);
    }

    return btn;
}

void GameView::setButtonCallback(lv_obj_t* btn, const char* callbackName)
{
    // TODO: Register callback with Lua
    // For now, just store the name for Lua to retrieve
}

void GameView::removeButton(lv_obj_t* btn)
{
    if (btn)
        lv_obj_del(btn);
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
