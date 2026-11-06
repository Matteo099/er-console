#pragma once

#include "systems/ui/Scene.h"
#include "scenes/network/NetworkView.h"

class NetworkScene : public Scene
{
public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;
    void onPreferencesChanged() override { portalDataReady = false; refreshNetworkStatus(); }
    View& getView() override { return view; }
    SceneRequest getRequest() const override;
    bool handleUIEvent(const UIEvent& event) override;

private:
    uint32_t lastRefreshMs = 0;
    bool portalDataReady = false;
    void refreshNetworkStatus();

private:
    NetworkView view;
    SceneRequest request = SceneRequest();
    class NetworkSystem* network = nullptr;
    class PortalSystem* portal = nullptr;
};
