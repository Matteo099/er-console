#pragma once

#include "systems/ui/View.h"
#include <string>
#include <cstdint>

class GameView : public View
{
public:
    /* View lifecycle */
    void create() override;
    void update() override;
    void destroy() override;

    /* View API (called by Scene) */
    void setGameTitle(const std::string &title);
    void setTime(uint32_t seconds);
    void setRunning(bool running);

    /* Signals (polled by Scene) */
    bool startPressed = false;
    bool cancelPressed = false;
    bool infoPressed = false;
    bool settingsPressed = false;

    lv_obj_t *getScreen() override { return root; };

private:
    /* Root */
    lv_obj_t *root = nullptr;

    /* Header */
    lv_obj_t *header = nullptr;
    lv_obj_t *titleLabel = nullptr;
    lv_obj_t *infoBtn = nullptr;
    lv_obj_t *settingsBtn = nullptr;

    lv_obj_t *brandImg = nullptr;
    lv_obj_t *container = nullptr;
    lv_obj_t *textLabel = nullptr;

    /* Timer */
    lv_obj_t *timerLabel = nullptr;

    /* Bottom buttons */
    lv_obj_t *startPauseBtn = nullptr;
    lv_obj_t *startPauseLabel = nullptr;
    lv_obj_t *cancelBtn = nullptr;

    /* Event callbacks */
    static void onStartPause(lv_event_t *e);
    static void onCancel(lv_event_t *e);
    static void onInfo(lv_event_t *e);
    static void onSettings(lv_event_t *e);
};
