#include "NetworkScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "core/SystemEventBus.h"
#include "systems/network/NetworkSystem.h"
#include "systems/portal/PortalSystem.h"
#include "config/Console.h"
#include <Arduino.h>

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
    if (network && millis() - lastRefreshMs >= 1000)
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
        request = SceneRequest(SceneChange::GoToSettingsSystem, SceneTransition::SlideRight);
        return true;
    }

    return false;
}

void NetworkScene::refreshNetworkStatus()
{
    lastRefreshMs = millis();
    if (!network)
    {
        view.setStatus("Servizio rete non disponibile", "", "");
        return;
    }

    const auto& status = network->getStatus();
    std::string label;
    switch (status.state)
    {
    case NetworkSystem::State::Connected:
        label = "Connessa";
        break;
    case NetworkSystem::State::Connecting:
        label = "Connessione in corso...";
        break;
    case NetworkSystem::State::Disconnected:
        label = "Non connessa";
        break;
    case NetworkSystem::State::Error:
        label = "Connessione non riuscita";
        break;
    default:
        label = "In attesa";
        break;
    }

    view.setStatus(label, status.ssid, status.ip);
#if WIFI_ENABLED && PORTAL_ENABLED
    const bool running = portal && portal->isRunning();
    if (running && !portalDataReady)
    {
        view.setPortalInfo(portal->portalUrl(), portal->wifiQrPayload());
        portalDataReady = true;
    }
    else if (!running)
    {
        view.setPortalInfo("", "");
        portalDataReady = false;
    }
#endif
}
