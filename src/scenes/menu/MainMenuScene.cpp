#include "MainMenuScene.h"

#include "MainMenuAction.h"

void MainMenuScene::onEnter()
{
    view.create();
}

void MainMenuScene::onUpdate()
{
    view.update();
    switch (view.consumeAction())
    {
    case MainMenuAction::NewGame:
        request = SceneRequest(SceneChange::GoToGameList);
        break;

    case MainMenuAction::Settings:
        request = SceneRequest(SceneChange::GoToSettings);
        break;

    case MainMenuAction::About:
        request = SceneRequest(SceneChange::GoToAbout);
        break;

    default:
        break;
    }
}

void MainMenuScene::onExit()
{
    view.destroy();
}

SceneRequest MainMenuScene::getRequest() const
{
    return request;
}
