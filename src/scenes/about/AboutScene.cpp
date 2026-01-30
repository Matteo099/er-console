#include "AboutScene.h"

void AboutScene::onEnter()
{
}

void AboutScene::onUpdate()
{
    view.update();
}

void AboutScene::onExit()
{
    // handled by SceneManager → view.destroy()
}

SceneRequest AboutScene::getRequest() const
{
    return SceneRequest::None;
}
