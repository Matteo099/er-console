#include "UIManager.h"

#include "log/Logger.h"
#include "scenes/boot/BootScene.h"
#include "drivers/display/TFTDisplay.h"
#include "drivers/touch/XPT2046Touch.h"
#include "drivers/storage/ESP32SDCardDriver.h"
#include "systems/storage/StorageSystem.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"
#include "config/BoardConfig.h"

void UIManager::init(void *params)
{
    Log::info(Log::UI_TAG, "Initializing UI Manager");

    static TFTDisplay display;
    static XPT2046Touch touch;

    display.init();
    // touch.init();

    // 1. Initialize LVGL core + display + touch
    lvgl.init(&display, &touch);

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
