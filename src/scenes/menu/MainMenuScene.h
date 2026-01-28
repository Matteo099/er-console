#pragma once

#include "systems/ui/Scene.h"
#include "scenes/menu/MainMenuView.h"

class MainMenuScene : public Scene
{
public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneTransition getTransition() const override
    {
        return SceneTransition::Fade;
    }

private:
    MainMenuView view;
};
