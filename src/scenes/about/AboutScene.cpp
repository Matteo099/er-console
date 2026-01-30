#include "AboutScene.h"

#include "AboutAction.h"

void AboutScene::onEnter()
{
    view.create();
}

void AboutScene::onUpdate()
{
    switch (view.consumeAction())
    {
    case AboutAction::Back:
        request = SceneRequest::GoToMainMenu;
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
