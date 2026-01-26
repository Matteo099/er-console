#pragma once

#include "core/SystemService.h"
#include "systems/ui/lvgl/LVGLSystem.h"

class UIManager : public SystemService
{
private:
    LVGLSystem *lvgl;

public:
    void init() override;
    void update() override;
};