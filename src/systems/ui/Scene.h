#pragma once

#include "systems/ui/View.h"

class Scene
{
public:
    virtual ~Scene() {}
    virtual void onEnter() = 0;
    virtual void onUpdate() = 0;
    virtual void onExit() = 0;

    virtual View getView() = 0;
};