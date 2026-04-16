#pragma once

#include "systems/ui/Scene.h"
#include "scenes/monitor/MonitorView.h"

class MonitorScene : public Scene
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
    void refreshTelemetry();
    static const char* formatBytes(uint64_t bytes);

private:
    MonitorView view;
    SceneRequest request = SceneRequest();
    class InputSystem* input = nullptr;
    class StorageSystem* storage = nullptr;
};
