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
    pauseOverlay = Container::create(root, 90, 70, LV_ALIGN_CENTER, 0, -40);
    lv_obj_set_style_bg_opa(pauseOverlay, LV_OPA_TRANSP, 0);
    pauseIcon = Label::create(pauseOverlay, LV_SYMBOL_PAUSE, &lv_font_roboto_mono_28, LV_TEXT_ALIGN_CENTER);
    Align::setPosition(pauseIcon, LV_ALIGN_CENTER, 0, 0);
    Label::setTextColor(pauseIcon, lv_color_hex(0xFFFF00));
    lv_obj_add_flag(pauseOverlay, LV_OBJ_FLAG_HIDDEN);

    // Bottom row container
    auto footer = lv_obj_create(root);
    lv_obj_remove_style_all(footer);
    lv_obj_set_size(footer, LV_PCT(100), 60);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_flex_flow(footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footer, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);       
    /* objects/info button*/
    objectsBtn = Button::create(footer, nullptr, onObjects, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_BELL); 
    /* cancel */
    cancelBtn = Button::create(footer, nullptr, onCancel, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_CLOSE); 
    /* start/pause */
    startPauseBtn = Button::create(footer, nullptr, onStartPause, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_PLAY);
    startPauseIcon = lv_obj_get_child(startPauseBtn, 0);
    startPauseLabel = lv_obj_get_child(startPauseBtn, 1);
}

void MainSubview::destroy()
{
    root = nullptr;
    timerLabel = nullptr;

    startPauseBtn = nullptr;
    startPauseIcon = nullptr;
    startPauseLabel = nullptr;
    objectsBtn = nullptr;
    cancelBtn = nullptr;

    pauseOverlay = nullptr;
    pauseIcon = nullptr;
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
    uint32_t now = lv_tick_get();

    /* timer warning blink */
    if (remainingSeconds <= WARNING_THRESHOLD_SEC)
    {
        if (now - lastBlinkMs >= 500)
        {
            lastBlinkMs = now;
            warningBlinkState = !warningBlinkState;
            lv_obj_set_style_text_color(timerLabel, warningBlinkState ? lv_color_hex(0xFF0000) : lv_color_hex(0xFFFFFF), 0);
        }
        if (!tickingClockAudioStarted)
        {
            sendAudioEvent(AUDIO_TICKING_CLOCK_FILE, AudioEventType::PLAY_LOOP_WAV);
            tickingClockAudioStarted = true;
        }
    }

    /* pause overlay blink */
    if (paused)
    {
        if (now - lastPauseBlinkMs >= 500)
        {
            lastPauseBlinkMs = now;
            pauseBlinkState = !pauseBlinkState;
            lv_obj_set_style_opa(pauseIcon, pauseBlinkState ? LV_OPA_COVER : LV_OPA_30, 0);
        }
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
    {
        lv_label_set_text(startPauseIcon, running ? LV_SYMBOL_PAUSE : LV_SYMBOL_PLAY);
    }

    if (startPauseLabel)
    {
        lv_label_set_text(startPauseLabel, running ? "Pause" : "Start");
    }

    paused = !running;
    if (pauseOverlay)
    {
        if (paused)
        {
            lv_obj_clear_flag(pauseOverlay, LV_OBJ_FLAG_HIDDEN);
            pauseBlinkState = true;
            lv_obj_set_style_opa(pauseIcon, LV_OPA_COVER, 0);
        }
        else
        {
            lv_obj_add_flag(pauseOverlay, LV_OBJ_FLAG_HIDDEN);
        }
    }
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

void MainSubview::onObjects(lv_event_t *e)
{
    auto *view =
        static_cast<MainSubview *>(lv_event_get_user_data(e));

    if (!view)
        return;

    if (view->eventCallback)
        view->eventCallback("objects");
}