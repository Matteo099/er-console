#include "SettingsScene.h"

void SettingsScene::onEnter()
{
}

void SettingsScene::onUpdate()
{
    view.update();
}

void SettingsScene::onExit()
{
    // handled by SceneManager → view.destroy()
}

SceneRequest SettingsScene::getRequest() const
{
    return SceneRequest::None;
}
