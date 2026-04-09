#pragma once

#include "systems/ui/Scene.h"
#include "scenes/sample/SampleView.h"

class SampleScene : public Scene
{
public:    
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::Fade; }
    SceneRequest getRequest() const override;

    // UI event handling
    bool handleUIEvent(const UIEvent& event) override;

private:
    SampleView view;
};
