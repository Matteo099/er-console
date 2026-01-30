#pragma once

#include "systems/ui/Scene.h"
#include "scenes/game/GameView.h"

class GameScene : public Scene
{
public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View &getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::SlideLeft; }
    SceneRequest getRequest() const override;

private:
    GameView view;
    SceneRequest request = SceneRequest();
};
