#include "NetworkScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "core/SystemEventBus.h"
#include "systems/network/NetworkSystem.h"
#include "systems/portal/PortalSystem.h"

void NetworkScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering NetworkScene");
    view.setEventHandler(this);
    view.create();

    network = static_cast<NetworkSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    portal = static_cast<PortalSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_PORTAL));
    refreshNetworkStatus();
}

void NetworkScene::onUpdate()
{
    if (network)
        refreshNetworkStatus();

    view.update();
}

void NetworkScene::onExit()
{
    view.destroy();
}

SceneRequest NetworkScene::getRequest() const
{
    return request;
}

bool NetworkScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "portal_qr")
    {
        view.toggleQrOverlay();
        refreshNetworkStatus();
        return true;
    }

    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToSettings);
        return true;
    }

    return false;
}

void NetworkScene::refreshNetworkStatus()
{
    if (!network)
    {
        view.setStatus("Network service unavailable", "—", "—");
        return;
    }

    const auto& status = network->getStatus();
    std::string label = "Status: ";
    switch (status.state)
    {
    case NetworkSystem::State::Connected:
        label += "Connected";
        break;
    case NetworkSystem::State::Connecting:
        label += "Connecting...";
        break;
    case NetworkSystem::State::Disconnected:
        label += "Disconnected";
        break;
    case NetworkSystem::State::Error:
        label += "Error";
        break;
    default:
        label += "Idle";
        break;
    }

    if (!status.message.empty())
        label += " - " + status.message;

    std::string portalUrl = portal ? portal->portalUrl() : std::string("http://192.168.4.1/");
    std::string qrData = portal ? portal->wifiQrPayload() : std::string("WIFI:S:ER-Console-Setup;T:nopass;P:;H:false;;");

    view.setStatus(label, status.ssid, status.ip);
    view.setPortalInfo(portalUrl, qrData);
}
