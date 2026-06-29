#include "MainSubview.h"

#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/Align.h"
#include "systems/ui/UIEvents.h"
#include "config/Assets.h"

void MainSubview::create(lv_obj_t *parent)
{
    root = Container::create(parent, LV_PCT(100), LV_PCT(100));
    timerLabel = Label::create(root, "00:00:00", &lv_font_roboto_mono_48, LV_TEXT_ALIGN_CENTER);
    Align::setPosition(timerLabel, LV_ALIGN_CENTER, 0, -40);
    gameContentArea = Container::create(root, LV_PCT(100), LV_PCT(100) - 96);
    lv_obj_set_pos(gameContentArea, 0, 48);
    lv_obj_set_style_bg_color(gameContentArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(gameContentArea, LV_OPA_COVER, 0);
    cancelBtn = Button::create(root, "Cancel", onCancel, this, 120, 44, LV_ALIGN_BOTTOM_LEFT, 20, -20, LV_SYMBOL_CLOSE);
    startPauseBtn = Button::create(root, "Start", onStartPause, this, 120, 44, LV_ALIGN_BOTTOM_RIGHT, -20, -20, LV_SYMBOL_PLAY);
    startPauseIcon = lv_obj_get_child(startPauseBtn, 0);
    startPauseLabel = lv_obj_get_child(startPauseBtn, 1);
}

void MainSubview::destroy()
{
    root = nullptr;
    timerLabel = nullptr;
    gameContentArea = nullptr;

    startPauseBtn = nullptr;
    startPauseIcon = nullptr;
    startPauseLabel = nullptr;

    cancelBtn = nullptr;
}

void MainSubview::show()
{
    if (root)
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void MainSubview::hide()
{
    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void MainSubview::update()
{
    if (remainingSeconds > WARNING_THRESHOLD_SEC)
        return;

    if (!tickingClockAudioStarted)
    {
        sendAudioEvent(
            AUDIO_TICKING_CLOCK_FILE,
            AudioEventType::PLAY_LOOP_WAV);

        tickingClockAudioStarted = true;
    }

    uint32_t now = lv_tick_get();

    if (now - lastBlinkMs >= 500)
    {
        lastBlinkMs = now;
        warningBlinkState = !warningBlinkState;

        lv_obj_set_style_text_color(
            timerLabel,
            warningBlinkState
                ? lv_color_hex(0xFF0000)
                : lv_color_hex(0xFFFFFF),
            0);
    }
}

void MainSubview::setTime(uint32_t seconds)
{
    remainingSeconds = seconds;

    char buf[16];

    snprintf(
        buf,
        sizeof(buf),
        "%02lu:%02lu:%02lu",
        seconds / 3600,
        (seconds % 3600) / 60,
        seconds % 60);

    lv_label_set_text(timerLabel, buf);
}

void MainSubview::setRunning(bool running)
{
    if (startPauseIcon)
        lv_label_set_text(
            startPauseIcon,
            running ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);

    if (startPauseLabel)
        lv_label_set_text(
            startPauseLabel,
            running ? "Pause" : "Start");
}

void MainSubview::onStartPause(lv_event_t *e)
{
    auto *view = static_cast<MainSubview *>(lv_event_get_user_data(e));

    if (!view)
        return;

    if (view->eventCallback)
        view->eventCallback("start_pause");
}

void MainSubview::onCancel(lv_event_t *e)
{
    auto *view = static_cast<MainSubview *>(lv_event_get_user_data(e));

    if (!view)
        return;

    if (view->eventCallback)
        view->eventCallback("cancel");
}