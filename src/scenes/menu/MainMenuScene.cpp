#include "MainMenuScene.h"
#include "log/Logger.h"

#include "MainMenuAction.h"

void MainMenuScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering MainMenuScene");
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
}

void MainMenuScene::onExit()
{
}

SceneRequest MainMenuScene::getRequest() const
{
    return request;
}
