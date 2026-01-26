#pragma once
#include "ui/Scene.h"

class SceneManager {
public:
    void init();
    void update();
    void changeScene(Scene* next);
};