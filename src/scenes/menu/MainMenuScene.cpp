#include "MainMenuScene.h"

void MainMenuScene::onEnter()
{
    view.create();
}

void MainMenuScene::onUpdate()
{
    view.update();
}

void MainMenuScene::onExit()
{
    view.destroy();
}
