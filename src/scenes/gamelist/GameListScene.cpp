#include "GameListScene.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "scenes/menu/MainMenuScene.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"

void GameListScene::onEnter()
{
}

void GameListScene::onUpdate()
{
    view.update();
}

void GameListScene::onExit()
{
    // handled by SceneManager → view.destroy()
}

SceneRequest GameListScene::getRequest() const
{
    return SceneRequest::None;
}
