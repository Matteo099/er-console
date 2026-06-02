#include "SettingsScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "core/SystemEventBus.h"
#include "systems/network/NetworkSystem.h"

void SettingsScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering SettingsScene");
    view.setEventHandler(this);
    view.setSoundEnabled(soundEnabled);
    view.create();

    network = static_cast<NetworkSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    if (network)
    {
        const auto& status = network->getStatus();
        view.setWiFiStatus(status.message);
    }
}

void SettingsScene::onUpdate()
{
    if (network)
    {
        view.setWiFiStatus(network->getStatus().message);
    }
    view.update();
}

void SettingsScene::onExit()
{
    view.destroy();
}

SceneRequest SettingsScene::getRequest() const
{
    return request;
}

bool SettingsScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "toggle_sound")
    {
        soundEnabled = !soundEnabled;
        view.setSoundEnabled(soundEnabled);
        return true;
    }

    if (clicked.buttonId == "wifi_settings")
    {
        request = SceneRequest(SceneChange::GoToNetwork, SceneTransition::SlideLeft);
        return true;
    }

    if (clicked.buttonId == "monitor")
    {
        request = SceneRequest(SceneChange::GoToMonitor, SceneTransition::SlideLeft);
        return true;
    }

    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToMainMenu, SceneTransition::SlideRight);
        return true;
    }

    return false;
}
