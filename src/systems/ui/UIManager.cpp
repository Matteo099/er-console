#include "UIManager.h"

#include "log/Logger.h"
#include "scenes/boot/BootScene.h"
#include "scenes/sample/SampleScene.h"
#include "drivers/display/TFTDisplay.h"
#include "drivers/storage/ESP32SDCardDriver.h"
#include "systems/storage/StorageSystem.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"
#include "config/BoardConfig.h"

void UIManager::init(void *params)
{
    Log::info(Log::UI_TAG, "Initializing UI Manager");

    static TFTDisplay display;

    // 1. Initialize display (LovyanGFX)
    display.init();

    // 2. Initialize LVGL core 
    lvgl.init(&display, &display);

    // 3. Initialize scene manager
    scenes.init();

    // 4. Load initial scene (BOOT)
    scenes.changeScene(new SampleScene());
    // scenes.changeScene(new BootScene());

    Log::info(Log::UI_TAG, "UI Manager ready");
}

void UIManager::update()
{
    // Advance scene logic (non-blocking)
    scenes.update();

    // Advance LVGL timers & rendering
    lvgl.update();
}
