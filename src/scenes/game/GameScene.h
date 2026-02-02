#pragma once

#include "systems/ui/Scene.h"
#include "GameView.h"
#include "GameState.h"

class GameScene : public Scene
{
public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::SlideLeft; }
    SceneRequest getRequest() const override;

private:
    GameView view;

    GameState state = GameState::Ready;
    uint32_t remainingSec = 3600;
    uint32_t lastTickMs = 0;

    SceneRequest request =SceneRequest();
};
