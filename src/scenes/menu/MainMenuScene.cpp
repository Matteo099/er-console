#include "MainMenuScene.h"
#include "log/Logger.h"

#include "MainMenuAction.h"

void MainMenuScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering MainMenuScene");
    view.create();
}

void MainMenuScene::onUpdate()
{
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
    view.update();
}

void MainMenuScene::onExit()
{
    view.destroy();
}

SceneRequest MainMenuScene::getRequest() const
{
    return request;
}
