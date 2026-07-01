#pragma once

#include <lvgl.h>
#include <string>
#include <cstdint>
#include "../GameSubview.h"
#include "systems/audio/AudioHandle.h"

class MainSubview
{
public:
    void create(lv_obj_t *parent);
    void destroy();

    void show();
    void hide();

    void update();

    /* Game state */
    void setTime(uint32_t seconds);
    void setRunning(bool running);
    void setEventCallback(EventCallback callback)
    {
        eventCallback = std::move(callback);
    }

private:
    EventCallback eventCallback;

    static constexpr uint32_t WARNING_THRESHOLD_SEC = 60;

    lv_obj_t *root = nullptr;

    lv_obj_t *timerLabel = nullptr;

    lv_obj_t *startPauseBtn = nullptr;
    lv_obj_t *startPauseIcon = nullptr;
    lv_obj_t *startPauseLabel = nullptr;

    lv_obj_t *cancelBtn = nullptr;

    bool warningBlinkState = false;
    bool tickingClockAudioStarted = false;
    AudioHandle tickingAudioHandle;

    uint32_t remainingSeconds = 0;
    uint32_t lastBlinkMs = 0;

    lv_obj_t *pauseOverlay = nullptr;
    lv_obj_t *pauseIcon = nullptr;

    bool paused = false;
    bool pauseBlinkState = false;
    uint32_t lastPauseBlinkMs = 0;

    lv_obj_t *objectsBtn = nullptr;

    static void onObjects(lv_event_t *e);
    static void onStartPause(lv_event_t *e);
    static void onCancel(lv_event_t *e);
};