#pragma once

#include "systems/ui/Scene.h"
#include "scenes/monitor/MonitorView.h"

class MonitorScene : public Scene {
  public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    void onPreferencesChanged() override {
        refreshTelemetry();
    }

    View& getView() override {
        return view;
    }

    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent& event) override;

  private:
    void refreshTelemetry();

  private:
    MonitorView view;
    SceneRequest request = SceneRequest();
    class InputSystem* input = nullptr;
    class StorageSystem* storage = nullptr;
    uint32_t lastRefreshMs = 0;
};
