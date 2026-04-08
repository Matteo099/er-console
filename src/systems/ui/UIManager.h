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

public:
    void init(void *params = nullptr) override;
    void update() override;

    SceneManager &sceneManager() { return scenes; }
};