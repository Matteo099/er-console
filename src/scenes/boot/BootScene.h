#pragma once

#include "systems/ui/Scene.h"
#include "scenes/boot/BootView.h"

class BootScene : public Scene
{
public:
    BootScene();
    
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::Fade; }

private:
    BootView view;
    bool imagesLoaded;
};
