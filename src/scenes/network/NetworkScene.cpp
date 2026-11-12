#include "NetworkScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "core/SystemEventBus.h"
#include "systems/network/NetworkSystem.h"
#include "systems/portal/PortalSystem.h"
#include "config/Console.h"
#include "services/DeviceConfigRepository.h"
#include "systems/ui/localization/I18n.h"
#include <Arduino.h>

void NetworkScene::onEnter() {
    Log::info(Log::SCENE_TAG, "Entering NetworkScene");
    view.setEventHandler(this);
    view.create();
    DeviceConfig config;
    DeviceConfigRepository().loadCached(config);
    view.setCredentials(config.wifi.ssid, config.wifi.password);

    network = static_cast<NetworkSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    portal = static_cast<PortalSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_PORTAL));
    refreshNetworkStatus();
    if (network)
        network->requestScan();
    refreshScan(true);
}

void NetworkScene::onUpdate() {
    if (network && millis() - lastRefreshMs >= 1000)
        refreshNetworkStatus();

    view.update();
    refreshScan();
}

void NetworkScene::onExit() {
    view.destroy();
}

SceneRequest NetworkScene::getRequest() const {
    return request;
}

bool NetworkScene::handleUIEvent(const UIEvent& event) {
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "scan_networks") {
        if (network)
            network->requestScan();
        refreshScan(true);
        return true;
    }
    if (clicked.buttonId == "save_network") {
        if (view.requiresPassword() && view.secret().empty())
            view.setMessage(Text::PasswordRequired);
        else if (view.ssid().empty() ||
                 !DeviceConfigRepository::validNetwork(view.ssid(), view.secret()))
            view.setMessage(Text::NetworkInvalid);
        else if (!DeviceConfigRepository().saveNetworkPreferences(view.ssid(), view.secret()))
            view.setMessage(Text::NetworkSaveFailed);
        else {
            view.setMessage(WIFI_ENABLED ? Text::None : Text::NetworkSaved);
            if (network) {
                network->setCredentials(view.ssid(), view.secret());
#if WIFI_ENABLED
                network->connect();
#endif
            }
            refreshNetworkStatus();
#if WIFI_ENABLED
            view.showStatus();
#endif
            return true;
        }
        refreshNetworkStatus();
        return true;
    }
    if (clicked.buttonId == "disconnect_network") {
        if (network)
            network->disconnect();
        refreshNetworkStatus();
        return true;
    }
    if (clicked.buttonId == "portal_qr") {
        view.toggleQrOverlay();
        refreshNetworkStatus();
        return true;
    }

    if (clicked.buttonId == "back") {
        request = SceneRequest(SceneChange::GoToSettingsSystem, SceneTransition::SlideRight);
        return true;
    }

    return false;
}

void NetworkScene::refreshScan(bool force) {
    WifiScanSnapshot snapshot;
    if (network) {
        if (!network->scanSnapshot(force ? 0 : scanRevision, snapshot))
            return;
    } else
        snapshot.state = WifiScanState::Error;
    if (force || snapshot.revision != scanRevision) {
        scanRevision = snapshot.revision;
        view.setScanResults(snapshot);
    }
}

void NetworkScene::refreshNetworkStatus() {
    lastRefreshMs = millis();
    if (!network) {
        view.setStatus(tr(Text::NetworkUnavailable), "", "");
        return;
    }

    const auto& status = network->getStatus();
    std::string label;
    switch (status.state) {
        case NetworkSystem::State::Connected:
            label = tr(Text::Connected);
            break;
        case NetworkSystem::State::Connecting:
            label = tr(Text::Connecting);
            break;
        case NetworkSystem::State::Disconnected:
            label = tr(Text::Disconnected);
            break;
        case NetworkSystem::State::Error:
            label = tr(status.timedOut ? Text::ConnectionTimeout : Text::ConnectionFailed);
            break;
        default:
            label = tr(Text::Waiting);
            break;
    }

#if !WIFI_ENABLED
    label = tr(Text::WifiDisabled);
#endif
    view.setStatus(label, status.ssid, status.ip, status.signal);
#if WIFI_ENABLED && PORTAL_ENABLED
    const bool running = portal && portal->isRunning();
    if (running && !portalDataReady) {
        view.setPortalInfo(portal->portalUrl(), portal->wifiQrPayload());
        portalDataReady = true;
    } else if (!running) {
        view.setPortalInfo("", "");
        portalDataReady = false;
    }
#endif
}
