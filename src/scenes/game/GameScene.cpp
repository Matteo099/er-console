#include "GameScene.h"
#include "log/Logger.h"

void GameScene::onEnter()
{
<<<<<<< HEAD
    view.create();
    view.setTime(remainingSec);
    view.setRunning(false);
    view.setTime(remainingSec);
    view.setRunning(false);
=======
    Log::info(Log::SCENE_TAG, "Entering GameScene");
>>>>>>> 4b9c6e7 (add logging to various drivers and scenes for better debugging and tracking)
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
}

SceneRequest GameScene::getRequest() const
{
    return request;
}
