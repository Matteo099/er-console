#pragma once

#include "systems/ui/Scene.h"
#include "scenes/About/AboutView.h"

class AboutScene : public Scene
{
public:    
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::Fade; }
    SceneRequest getRequest() const override;

private:
    AboutView view;
};
