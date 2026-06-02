#pragma once

#include "core/SystemService.h"
#include "systems/ui/Scene.h"

class SceneManager : public SystemService
{
public:
    void init(void *params = nullptr) override;
    void update() override;

    void changeScene(Scene* next, SceneTransition transition);

private:
    Scene* current = nullptr;
    Scene* pendingDelete = nullptr;
    uint32_t pendingDeleteAtTick = 0;
};
