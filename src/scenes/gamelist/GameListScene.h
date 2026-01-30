#pragma once

#include "systems/ui/Scene.h"
#include "scenes/gamelist/GameListView.h"

class GameListScene : public Scene
{
public:    
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneTransition getTransition() const override { return SceneTransition::Fade; }
    SceneRequest getRequest() const override;

private:
    GameListView view;
};
