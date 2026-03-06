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
    void setGameTitle(const std::string& title);
    void setTime(uint32_t seconds);
    void setRunning(bool running);

    /* Loading and error handling */
    void showLoadingBar();
    void hideLoadingBar();
    void setErrorMessage(const std::string& message);

    /* Lua API - Display control */
    void clearDisplay();
    void drawText(int x, int y, const char* text, int fontSize = 16, uint32_t color = 0xFFFFFF);
    void drawRect(int x, int y, int width, int height, uint32_t color = 0xFFFFFF, bool fill = false);
    void drawLine(int x1, int y1, int x2, int y2, uint32_t color = 0xFFFFFF);

    /* Lua API - UI Components */
    lv_obj_t* createButton(int x, int y, int width, int height, const char* text);
    void setButtonCallback(lv_obj_t* btn, const char* callbackName);
    void removeButton(lv_obj_t* btn);

    /* Signals (polled by Scene) */
    bool startPressed = false;
    bool cancelPressed = false;
    bool infoPressed = false;
    bool settingsPressed = false;

    lv_obj_t *getScreen() override { return root; };

private:
    /* Root */
    lv_obj_t* root = nullptr;

    /* Header */
    lv_obj_t* header = nullptr;
    lv_obj_t* titleLabel = nullptr;
    lv_obj_t* infoBtn = nullptr;
    lv_obj_t* settingsBtn = nullptr;

    /* Timer */
    lv_obj_t* timerLabel = nullptr;

    /* Loading */
    lv_obj_t* loadingContainer = nullptr;
    lv_obj_t* loadingBar = nullptr;
    lv_obj_t* loadingLabel = nullptr;

    /* Error message */
    lv_obj_t* errorLabel = nullptr;

    /* Game content area (for Lua-drawn content) */
    lv_obj_t* gameContentArea = nullptr;

    /* Bottom buttons */
    lv_obj_t* startPauseBtn = nullptr;
    lv_obj_t* startPauseLabel = nullptr;
    lv_obj_t* cancelBtn = nullptr;

    /* Event callbacks */
    static void onStartPause(lv_event_t* e);
    static void onCancel(lv_event_t* e);
    static void onInfo(lv_event_t* e);
    static void onSettings(lv_event_t* e);
};
