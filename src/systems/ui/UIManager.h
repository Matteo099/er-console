#pragma once

#include "core/SystemService.h"
#include "systems/ui/lvgl/LVGLContext.h"
#include "systems/ui/SceneManager.h"

class UIManager : public SystemService
{
private:
    LVGLContext lvgl;
    SceneManager scenes;

public:
    void init() override;
    void update() override;
};