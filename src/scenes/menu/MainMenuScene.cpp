#include "MainMenuScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include "systems/network/NetworkSystem.h"
#include "systems/portal/PortalSystem.h"
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
#if WIFI_ENABLED
    auto *network = static_cast<NetworkSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    auto *portal = static_cast<PortalSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_PORTAL));
    auto state = MainMenuView::WiFiState::Disconnected;
    if (portal && portal->isRunning()) state = MainMenuView::WiFiState::AccessPoint;
    if (network)
    {
        switch (network->getStatus().state)
        {
        case NetworkSystem::State::Connected: state = MainMenuView::WiFiState::Connected; break;
        case NetworkSystem::State::Connecting: state = MainMenuView::WiFiState::Connecting; break;
        case NetworkSystem::State::Error:
            if (state != MainMenuView::WiFiState::AccessPoint) state = MainMenuView::WiFiState::Error;
            break;
        default: break;
        }
    }
    view.setWiFiState(state);
#endif
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
