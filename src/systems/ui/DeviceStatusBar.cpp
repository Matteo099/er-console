#include "DeviceStatusBar.h"
#include <Arduino.h>
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include "systems/network/NetworkSystem.h"
#include "systems/portal/PortalSystem.h"
#include "config/Console.h"

void DeviceStatusBar::attachTo(lv_obj_t *screen)
{
    root = Container::create(screen, Width, 22, LV_ALIGN_TOP_RIGHT, -12, 11);
    lv_obj_add_flag(root, LV_OBJ_FLAG_FLOATING);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_CLICKABLE);
    Container::setFlex(root, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(root, 7, 0);
    sdBadge = StatusBadge::create(root, LV_SYMBOL_DRIVE, "SD");
    wifiBadge = StatusBadge::create(root, LV_SYMBOL_WIFI, "OFF");
    previewing = false;
    update(true);
}

void DeviceStatusBar::update(bool force)
{
    if (!root) return;
    // Dialogs may move in front of the screen; keep status above their backdrop.
    auto *parent = lv_obj_get_parent(root);
    if (lv_obj_get_index(root) != int32_t(lv_obj_get_child_count(parent)) - 1) lv_obj_move_foreground(root);
    if (previewing || (!force && millis() - lastRefresh < 500)) return;
    lastRefresh = millis();
    auto &bus = SystemEventBus::instance();
    auto *storage = static_cast<StorageSystem *>(bus.getService(SystemEventBus::SERVICE_STORAGE));
    StorageState sd = StorageState::Unavailable;
    if (storage)
        switch (storage->health())
        {
        case StorageSystem::Health::Ready: sd = StorageState::Ready; break;
        case StorageSystem::Health::Degraded: sd = StorageState::Degraded; break;
        case StorageSystem::Health::Error: sd = StorageState::Error; break;
        case StorageSystem::Health::Unavailable: break;
        }
    WiFiState wifi = WiFiState::Disconnected;
#if WIFI_ENABLED
    auto *portal = static_cast<PortalSystem *>(bus.getService(SystemEventBus::SERVICE_PORTAL));
    auto *network = static_cast<NetworkSystem *>(bus.getService(SystemEventBus::SERVICE_NETWORK));
    if (portal && portal->isRunning()) wifi = WiFiState::AccessPoint;
    if (network)
        switch (network->getStatus().state)
        {
        case NetworkSystem::State::Connected: wifi = WiFiState::Connected; break;
        case NetworkSystem::State::Connecting: wifi = WiFiState::Connecting; break;
        case NetworkSystem::State::Error:
            if (wifi != WiFiState::AccessPoint) wifi = WiFiState::Error;
            break;
        default: break;
        }
#endif
    render(sd, wifi);
}

void DeviceStatusBar::preview(StorageState sd, WiFiState wifi)
{
    previewing = true;
    render(sd, wifi);
}

void DeviceStatusBar::render(StorageState sd, WiFiState wifi)
{
    const bool unavailable = sd == StorageState::Unavailable || sd == StorageState::Error;
    StatusBadge::set(sdBadge, sd == StorageState::Ready ? UIStatusTone::Good :
                     sd == StorageState::Degraded ? UIStatusTone::Warning : UIStatusTone::Error,
                     unavailable, sd == StorageState::Degraded ? "SD!" : "SD");
    const char *text = "OFF";
    UIStatusTone tone = UIStatusTone::Error;
    bool offline = true;
    switch (wifi)
    {
    case WiFiState::Connected: text = "OK"; tone = UIStatusTone::Good; offline = false; break;
    case WiFiState::AccessPoint: text = "AP"; tone = UIStatusTone::Good; offline = false; break;
    case WiFiState::Connecting: text = "..."; tone = UIStatusTone::Warning; offline = false; break;
    case WiFiState::Error: text = "!"; break;
    case WiFiState::Disconnected: break;
    }
    StatusBadge::set(wifiBadge, tone, offline, text);
}
