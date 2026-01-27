#include "UIManager.h"

#include "log/Logger.h"
#include "scenes/boot/BootScene.h"

void UIManager::init()
{
    Log::info(Log::UI_TAG, "Initializing UI Manager");

    // 1. Initialize LVGL core + display + touch
    lvgl.init();

    // 2. Initialize scene manager
    scenes.init();

    // 3. Load initial scene (BOOT)
    scenes.changeScene(new BootScene());

    Log::info(Log::UI_TAG, "UI Manager ready");
}

void UIManager::update()
{
    // Advance scene logic (non-blocking)
    scenes.update();

    // Advance LVGL timers & rendering
    lvgl.update();
}
