#pragma once

#include "core/SystemService.h"
#include "systems/ui/lvgl/LVGLContext.h"
#include "systems/ui/SceneManager.h"
#include "drivers/display/TFTDisplay.h"

class UIManager : public SystemService
{
private:
    TFTDisplay display;
    LVGLContext lvgl;
    SceneManager scenes;
    bool preferencesLoaded = false;
    uint32_t preferencesRevision = 0, localeRevision = 0;
    void loadPreferences();

public:
    void init(void *params = nullptr) override;
    void update() override;

    SceneManager &sceneManager() { return scenes; }
};
