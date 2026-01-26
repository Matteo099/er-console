#pragma once

#include "systems/ui/Scene.h"

class SceneManager {
public:
    void changeScene(Scene* next);
    void update();

private:
    Scene* current = nullptr;
    Scene* next = nullptr;
};
