#include "GameView.h"
#include "systems/ui/UIEvents.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/ProgressBar.h"
#include "systems/ui/components/Screen.h"
#include "Arduino.h"

void GameView::create()
{
    screen = Screen::create();
    lv_obj_set_size(screen, LV_PCT(100), LV_PCT(100));

    resultLabel = Label::create(screen, "", &lv_font_roboto_mono_28, LV_TEXT_ALIGN_CENTER, 240);
    Align::setPosition(resultLabel, LV_ALIGN_CENTER, 0, 30);
    Label::setTextColor(resultLabel, lv_color_hex(0xFFFFFF));
    lv_obj_add_flag(resultLabel, LV_OBJ_FLAG_HIDDEN);

    gameContentArea = Container::create(screen, LV_PCT(100), LV_PCT(100) - 96 - 48);
    lv_obj_set_pos(gameContentArea, 0, 48);
    lv_obj_set_style_bg_color(gameContentArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(gameContentArea, LV_OPA_COVER, 0);

    timerLabel = Label::create(screen, "00:00:00", &lv_font_roboto_mono_48, LV_TEXT_ALIGN_CENTER);
    Align::setPosition(timerLabel, LV_ALIGN_CENTER, 0, -40);

    loadingContainer = Container::create(screen, 200, 100, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(loadingContainer, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(loadingContainer, LV_OPA_COVER, 0);

    loadingLabel = Label::create(loadingContainer, "Loading game...", &lv_font_roboto_mono_14, LV_TEXT_ALIGN_CENTER, 200);
    Align::setPosition(loadingLabel, LV_ALIGN_TOP_MID, 0, 10);

    loadingBar = ProgressBar::create(loadingContainer, 160, 20);
    ProgressBar::setValue(loadingBar, 50, LV_ANIM_OFF);
    Align::alignTo(loadingBar, loadingLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_add_flag(loadingContainer, LV_OBJ_FLAG_HIDDEN);

    cancelBtn = Button::create(screen, "Cancel", onCancel, this, 120, 44, LV_ALIGN_BOTTOM_LEFT, 20, -20, LV_SYMBOL_CLOSE);

    startPauseBtn = Button::create(screen, "Start", onStartPause, this, 120, 44, LV_ALIGN_BOTTOM_RIGHT, -20, -20, LV_SYMBOL_PLAY);
    startPauseIcon = lv_obj_get_child(startPauseBtn, 0);
    startPauseLabel = lv_obj_get_child(startPauseBtn, 1);

    warningBlinkState = false;
    lastBlinkMs = 0;
    remainingSeconds = 0;
    tickingClockAudioStarted = false;
}

void GameView::update()
{
    if (!timerLabel)
        return;

    if (remainingSeconds > WARNING_THRESHOLD_SEC)
        return;

    if (tickingClockAudioStarted == false)
    {
        sendAudioEvent(AUDIO_TICKING_CLOCK_FILE, AudioEventType::PLAY_LOOP_WAV);
        tickingClockAudioStarted = true;
    }

    uint32_t now = lv_tick_get();

    if (now - lastBlinkMs >= 500)
    {
        warningBlinkState = !warningBlinkState;
        lastBlinkMs = now;

        lv_obj_set_style_text_color(
            timerLabel,
            warningBlinkState
                ? lv_color_hex(0xFF0000)
                : lv_color_hex(0xFFFFFF),
            0);
    }
}

void GameView::destroy()
{
    screen = nullptr;

    // Nullify all remaining pointers (defensive programming)
    // These are now safe to access even though objects are deleted
    timerLabel = nullptr;
    startPauseBtn = nullptr;
    startPauseLabel = nullptr;
    cancelBtn = nullptr;
    loadingContainer = nullptr;
    loadingBar = nullptr;
    loadingLabel = nullptr;
    errorLabel = nullptr;
    gameContentArea = nullptr;
    resultLabel = nullptr;
}

/* ================= API ================= */

void GameView::setTime(uint32_t seconds)
{
    remainingSeconds = seconds;
    if (!timerLabel)
        return;

    char buf[16];

    snprintf(buf,
             sizeof(buf),
             "%02lu:%02lu:%02lu",
             seconds / 3600,
             (seconds % 3600) / 60,
             seconds % 60);

    lv_label_set_text(timerLabel, buf);
}

void GameView::showGameOver()
{
    if (!resultLabel)
        return;

    lv_label_set_text(resultLabel, "GAME OVER");

    lv_obj_set_style_text_color(
        resultLabel,
        lv_color_hex(0xFF0000),
        0);

    lv_obj_clear_flag(resultLabel, LV_OBJ_FLAG_HIDDEN);

    setFinished(false);
}

void GameView::showVictory()
{
    if (!resultLabel)
        return;

    lv_label_set_text(resultLabel, "YOU WIN!");

    lv_obj_set_style_text_color(
        resultLabel,
        lv_color_hex(0x00FF00),
        0);

    lv_obj_clear_flag(resultLabel, LV_OBJ_FLAG_HIDDEN);

    setFinished(true);
}

void GameView::setFinished(bool success)
{
    if (startPauseBtn)
        lv_obj_add_flag(startPauseBtn, LV_OBJ_FLAG_HIDDEN);

    if (cancelBtn)
    {
        lv_obj_clear_flag(cancelBtn, LV_OBJ_FLAG_HIDDEN);

        auto icon = lv_obj_get_child(cancelBtn, 0);
        auto label = lv_obj_get_child(cancelBtn, 1);

        if (icon)
            lv_label_set_text(icon, LV_SYMBOL_CLOSE);

        if (label)
            lv_label_set_text(label, "Close");

        Align::setPosition(
            cancelBtn,
            LV_ALIGN_BOTTOM_MID,
            0,
            -20);
    }
}

void GameView::setRunning(bool running)
{
    if (startPauseIcon)
        lv_label_set_text(startPauseIcon, running ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
    if (startPauseLabel)
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

void GameView::setErrorMessage(const std::string &message)
{
    hideLoadingBar();

    if (!errorLabel)
    {
        errorLabel = Label::create(screen, "", nullptr, LV_TEXT_ALIGN_CENTER, LV_PCT(80));
        Align::setPosition(errorLabel, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_long_mode(errorLabel, LV_LABEL_LONG_WRAP);
        Label::setTextColor(errorLabel, lv_color_hex(0xFF0000));
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

void GameView::drawText(int x, int y, const char *text, int fontSize, uint32_t color)
{
    if (!gameContentArea || !text)
        return;

    lv_obj_t *label = Label::create(gameContentArea, text, nullptr, LV_TEXT_ALIGN_LEFT, LV_SIZE_CONTENT, x, y);
    Label::setTextColor(label, lv_color_hex(color));

    const lv_font_t *font = &lv_font_roboto_mono_14;
    if (fontSize >= 24)
        font = &lv_font_roboto_mono_28;
    else if (fontSize >= 20)
        font = &lv_font_roboto_mono_20;
    else if (fontSize >= 16)
        font = &lv_font_roboto_mono_16;

    Label::setFont(label, font);
}

void GameView::drawRect(int x, int y, int width, int height, uint32_t color, bool fill)
{
    if (!gameContentArea)
        return;

    lv_obj_t *rect = lv_obj_create(gameContentArea);
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

lv_obj_t *GameView::createButton(int x, int y, int width, int height, const char *text)
{
    if (!gameContentArea)
        return nullptr;

    lv_obj_t *btn = Button::create(gameContentArea, text ? text : "", nullptr, nullptr, width, height);
    lv_obj_set_pos(btn, x, y);

    return btn;
}

void GameView::setButtonCallback(lv_obj_t *btn, const char *callbackName)
{
    // TODO: Register callback with Lua
    // For now, just store the name for Lua to retrieve
}

void GameView::removeButton(lv_obj_t *btn)
{
    if (btn)
        lv_obj_del(btn);
}

/* ================= EVENTS ================= */

void GameView::onStartPause(lv_event_t *e)
{
    auto *view = static_cast<GameView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("start_pause"));
}

void GameView::onCancel(lv_event_t *e)
{
    auto *view = static_cast<GameView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("cancel"));
}