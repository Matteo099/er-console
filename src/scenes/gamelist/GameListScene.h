#pragma once

#include "../game/GameInfo.h"
#include "systems/ui/Scene.h"
#include "scenes/gamelist/GameListView.h"
#include <vector>

class GameListScene : public Scene
{
public:
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View &getView() override { return view; }
    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent& event) override;

private:
    GameListView view;
    // int selectedIndex = -1;
    GameInfo selectedGame;
    uint32_t catalogVersion = 0;
    SceneRequest request = SceneRequest();

    void refreshCatalog();
};
