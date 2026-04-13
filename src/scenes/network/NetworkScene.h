#pragma once

#include "systems/ui/Scene.h"
#include "scenes/network/NetworkView.h"

class NetworkScene : public Scene
{
public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;
    View& getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::SlideLeft; }
    SceneRequest getRequest() const override;
    bool handleUIEvent(const UIEvent& event) override;

private:
    void refreshNetworkStatus();

private:
    NetworkView view;
    SceneRequest request = SceneRequest();
    class NetworkSystem* network = nullptr;
    class PortalSystem* portal = nullptr;
};
