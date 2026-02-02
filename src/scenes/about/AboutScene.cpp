#include "AboutScene.h"
#include "log/Logger.h"

#include "AboutAction.h"

void AboutScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering AboutScene");
}

void AboutScene::onUpdate()
{
    switch (view.consumeAction())
    {
    case AboutAction::Back:
        request = SceneRequest(SceneChange::GoToMainMenu);
        break;

    default:
        break;
    }
}

void AboutScene::onExit()
{
}

SceneRequest AboutScene::getRequest() const
{
    return request;
}
