#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include "NetworkConnectionDiagnostics.h"
#include "core/SystemEventBus.h"
#include "services/DeviceConfigRepository.h"
#include "systems/network/NetworkSystem.h"
#include <Arduino.h>
#include <WiFi.h>
#include <nvs.h>
#include <cstring>

namespace
{
    // A committed backup survives resets during the startup tests. Credentials
    // stay on the device; serial commands expose only state and boolean flags.
    bool fixture(bool restore, bool empty)
    {
        nvs_handle_t backup;
        if (nvs_open("wifi-check", NVS_READWRITE, &backup) != ESP_OK) return false;
        uint8_t active=0, had=0;
        nvs_get_u8(backup,"active",&active);
        uint8_t bytes[512]; size_t size=sizeof(bytes);
        bool ok=true;
        if (restore)
        {
            if (active)
            {
                ok=nvs_get_u8(backup,"had",&had)==ESP_OK;
                if (ok && had) ok=nvs_get_blob(backup,"original",bytes,&size)==ESP_OK;
                nvs_handle_t target;
                if (ok && nvs_open("console-net",NVS_READWRITE,&target)==ESP_OK)
                {
                    auto result=had ? nvs_set_blob(target,"credentials",bytes,size):nvs_erase_key(target,"credentials");
                    ok=(result==ESP_OK || (!had && result==ESP_ERR_NVS_NOT_FOUND)) && nvs_commit(target)==ESP_OK;
                    nvs_close(target);
                }
                else ok=false;
                if (ok) ok=nvs_erase_all(backup)==ESP_OK && nvs_commit(backup)==ESP_OK;
            }
        }
        else
        {
            if (!active)
            {
                nvs_handle_t original;
                const auto opened=nvs_open("console-net",NVS_READONLY,&original);
                esp_err_t result=ESP_ERR_NVS_NOT_FOUND;
                if (opened==ESP_OK) { result=nvs_get_blob(original,"credentials",bytes,&size); nvs_close(original); }
                else if (opened!=ESP_ERR_NVS_NOT_FOUND) ok=false;
                had=result==ESP_OK;
                if (result!=ESP_OK && result!=ESP_ERR_NVS_NOT_FOUND) ok=false;
                if (ok && had) ok=nvs_set_blob(backup,"original",bytes,size)==ESP_OK;
                if (ok) ok=nvs_set_u8(backup,"had",had)==ESP_OK && nvs_set_u8(backup,"active",1)==ESP_OK && nvs_commit(backup)==ESP_OK;
            }
            if (ok)
            {
                char ssid[33]; snprintf(ssid,sizeof(ssid),"ER-absent-%012llx",ESP.getEfuseMac());
                ok=DeviceConfigRepository().saveNetworkPreferences(empty ? "":ssid,empty ? "":"diagnostic-only-pass");
            }
        }
        nvs_close(backup); return ok;
    }
}
void networkConnectionDiagnostics(const char *command)
{
    if (!std::strcmp(command,"ui wifi-log off") || !std::strcmp(command,"ui wifi-log on"))
    {
        // Arduino's ROM debug output bypasses the HardwareSerial mutex and can
        // split diagnostic responses. Direct Serial commands remain enabled.
        Serial.setDebugOutput(!std::strcmp(command,"ui wifi-log on"));
        Serial.println("@WIFILOG OK"); return;
    }
    if (!std::strcmp(command,"ui wifi-fixture empty") || !std::strcmp(command,"ui wifi-fixture unreachable") || !std::strcmp(command,"ui wifi-fixture restore"))
    {
        const bool ok=fixture(!std::strcmp(command,"ui wifi-fixture restore"),!std::strcmp(command,"ui wifi-fixture empty"));
        Serial.printf("@WIFIFIXTURE %s\n",ok ? "OK":"ERROR");
        return;
    }
    auto *network=static_cast<NetworkSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    if (!network) { Serial.println("@WIFI missing-service"); return; }
    const auto status=network->getStatus();
    Serial.printf("@WIFI state=%u configured=%u timeout=%u ip=%u sdk=%u\n",unsigned(status.state),unsigned(network->hasCredentials()),unsigned(status.timedOut),unsigned(!status.ip.empty()),unsigned(WiFi.status()));
}
#endif
