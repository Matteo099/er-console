#pragma once

#include "systems/ui/Scene.h"
#include "scenes/menu/MainMenuView.h"

class MainMenuScene : public Scene {
  public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override {
        return view;
    }

    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent& event) override;

  private:
    void refreshStatus();
    uint32_t lastStatusMs = 0;
    MainMenuView view;
    SceneRequest request = SceneRequest();
};
