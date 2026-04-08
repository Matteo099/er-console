#include "AboutScene.h"
#include "log/Logger.h"

#include "AboutAction.h"

void AboutScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering AboutScene");
    view.create();
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
    view.update();
}

void AboutScene::onExit()
{
    view.destroy();
}

SceneRequest AboutScene::getRequest() const
{
    return request;
}
