#pragma once

#include "systems/ui/Scene.h"
#include "scenes/settings/SettingsView.h"

class SettingsScene : public Scene
{
public:    
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent& event) override;

private:
    SettingsView view;
    SceneRequest request = SceneRequest();

    // ---- Settings state (logic-owned) ----
    bool soundEnabled = true;
    class NetworkSystem* network = nullptr;
};
