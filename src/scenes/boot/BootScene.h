#pragma once

#include "scenes/boot/BootView.h"
#include "systems/ui/Scene.h"

class BootScene : public Scene
{
public:
    BootScene();

    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View getView() override { return view; };

private:
    BootView view;
};
