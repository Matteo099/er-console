#pragma once

#include "systems/ui/Scene.h"
#include "scenes/Settings/SettingsView.h"

class SettingsScene : public Scene
{
public:    
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::SlideLeft; }
    SceneRequest getRequest() const override;

private:
    SettingsView view;
    SceneRequest request = SceneRequest::None;

    // ---- Settings state (logic-owned) ----
    bool soundEnabled = true;
};
