#pragma once

#include "core/SystemService.h"
#include "systems/ui/Scene.h"

class SceneManager : public SystemService
{
public:
    void init() override;
    void update() override;
    void changeScene(Scene *next);
};