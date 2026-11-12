#pragma once

#include "systems/ui/Scene.h"
#include "scenes/boot/BootView.h"

class BootScene : public Scene {
  public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override {
        return view;
    }

    SceneRequest getRequest() const override;

  private:
    BootView view;
    bool bootCompleted = false;
    short bootCompleteDelayTick = 30;
};
