#include "GameScene.h"

#include "GameAction.h"

void GameScene::onEnter()
{
    view.create();
}

void GameScene::onUpdate()
{
    switch (view.consumeAction())
    {
    case GameAction::Back:
        request = SceneRequest(SceneChange::GoToMainMenu);
        break;

    default:
        break;
    }
    view.update();
}

void GameScene::onExit()
{
    view.destroy();
}

SceneRequest GameScene::getRequest() const
{
    return request;
}
