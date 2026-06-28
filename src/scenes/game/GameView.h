#pragma once

#include "systems/ui/View.h"
#include <string>
#include <cstdint>

/**
 * @brief Game scene/view management
 *
 * Object Hierarchy & Cleanup Pattern:
 * - Follows Pattern 1 (Simple Root-Based Cleanup)
 * - Root object: `root` (Screen)
 * - All other objects are owned by root or its children
 * - Cleanup: On destroy(), delete root cascades to all children
 * - Dynamic objects (Lua-created): Tracked separately, deleted before root
 */
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

    /* Loading and error handling */
    void showLoadingBar();
    void hideLoadingBar();
    void setErrorMessage(const std::string &message);
    void showGameOver();
    void showVictory();
    void setFinished(bool success);

    /* Lua API - Display control */
    void clearDisplay();
    void drawText(int x, int y, const char *text, int fontSize = 16, uint32_t color = 0xFFFFFF);
    void drawRect(int x, int y, int width, int height, uint32_t color = 0xFFFFFF, bool fill = false);
    void drawLine(int x1, int y1, int x2, int y2, uint32_t color = 0xFFFFFF);

    /* Lua API - UI Components */
    lv_obj_t *createButton(int x, int y, int width, int height, const char *text);
    void setButtonCallback(lv_obj_t *btn, const char *callbackName);
    void removeButton(lv_obj_t *btn);

    lv_obj_t *getScreen() override { return screen; };

private:
    static constexpr uint32_t WARNING_THRESHOLD_SEC = 60;

    /* Root */
    lv_obj_t *screen = nullptr;

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

    /* Loading */
    lv_obj_t *loadingContainer = nullptr;
    lv_obj_t *loadingBar = nullptr;
    lv_obj_t *loadingLabel = nullptr;

    /* Error message */
    lv_obj_t *errorLabel = nullptr;

    /* End game overlay */
    lv_obj_t *resultLabel = nullptr;

    /* Timer warning */
    bool warningBlinkState = false;
    uint32_t lastBlinkMs = 0;
    bool gameFinished = false;
    uint32_t remainingSeconds = 0;

    /* Game content area (for Lua-drawn content) */
    lv_obj_t *gameContentArea = nullptr;

    /* Bottom buttons */
    lv_obj_t *startPauseBtn = nullptr;
    lv_obj_t *startPauseIcon = nullptr;
    lv_obj_t *startPauseLabel = nullptr;
    lv_obj_t *cancelBtn = nullptr;

    bool tickingClockAudioStarted = false;

    /* Event callbacks */
    static void onStartPause(lv_event_t *e);
    static void onCancel(lv_event_t *e);
    static void onInfo(lv_event_t *e);
    static void onSettings(lv_event_t *e);
};
