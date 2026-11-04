#include "MainMenuScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include "config/Console.h"
#include <Arduino.h>

void MainMenuScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering MainMenuScene");
    view.setEventHandler(this);
    view.create();
    refreshStatus();
}

void MainMenuScene::refreshStatus()
{
    lastStatusMs = millis();
    auto *storage = static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    auto sd = MainMenuView::StorageState::Unavailable;
    if (storage)
    {
        switch (storage->health())
        {
        case StorageSystem::Health::Ready: sd = MainMenuView::StorageState::Ready; break;
        case StorageSystem::Health::Degraded: sd = MainMenuView::StorageState::Degraded; break;
        case StorageSystem::Health::Error: sd = MainMenuView::StorageState::Error; break;
        case StorageSystem::Health::Unavailable: break;
        }
    }
    view.setStorageState(sd);
}

void MainMenuScene::onUpdate()
{
    if (millis() - lastStatusMs >= 500) refreshStatus();
    view.update();
}

void MainMenuScene::onExit()
{
    view.destroy();
}

SceneRequest MainMenuScene::getRequest() const
{
    return request;
}

bool MainMenuScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "new_game")
    {
        request = SceneRequest(SceneChange::GoToGameList, SceneTransition::SlideLeft);
        return true;
    }
    if (clicked.buttonId == "settings")
    {
        request = SceneRequest(SceneChange::GoToSettings, SceneTransition::SlideLeft);
        return true;
    }
    if (clicked.buttonId == "about")
    {
        request = SceneRequest(SceneChange::GoToAbout, SceneTransition::SlideLeft);
        return true;
    }

    return false;
}
