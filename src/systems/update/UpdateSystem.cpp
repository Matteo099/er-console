#include "UpdateSystem.h"
#include "UpdateMetadata.h"
#include "UpdateHttp.h"
#include "config/Updates.h"
#include "config/Console.h"
#include "config/Environment.h"
#include "core/SystemEventBus.h"
#include "systems/lua/LuaSystem.h"
#include "systems/audio/AudioSystem.h"
#include "log/Logger.h"
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <esp_ota_ops.h>
#include <esp_heap_caps.h>
#include <ctime>

std::atomic<bool> UpdateSystem::flashing{false};
namespace
{
    constexpr const char *TAG = "OTA", *NVS = "ota-updates";
    std::string apiBase() { return std::string("https://api.github.com/repos/") + OTA_GITHUB_OWNER + "/" + OTA_GITHUB_REPOSITORY; }
    std::string assetUrl(uint64_t id) { return apiBase() + "/releases/assets/" + std::to_string(id); }
    bool rollbackEnabled()
    {
#if defined(CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE) && CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE
        return true;
#else
        return false;
#endif
    }
    const esp_partition_t *updateSlot()
    {
        const auto *running = esp_ota_get_running_partition();
        const auto *next = esp_ota_get_next_update_partition(nullptr);
        const auto *a = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, nullptr);
        const auto *b = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, nullptr);
        if (!running || !next || !a || !b || next->address == running->address || a->size != b->size || ESP.getFlashChipSize() < 16 * 1024 * 1024)
            return nullptr;
        return next;
    }
    bool pendingBoot()
    {
        esp_ota_img_states_t state;
        const auto *running = esp_ota_get_running_partition();
        return running && esp_ota_get_state_partition(running, &state) == ESP_OK && state == ESP_OTA_IMG_PENDING_VERIFY;
    }
    void recordBoot(bool success)
    {
        Preferences preferences;
        if (!preferences.begin(NVS, false))
            return;
        if (success)
            preferences.putString("last", FW_VERSION);
        preferences.putUChar("result", success ? 2 : 3);
        preferences.remove("pending");
    }
    class NativeFlash : public UpdateFlash
    {
    public:
        explicit NativeFlash(const esp_partition_t *value, bool validateOnly = false) : partition(value), validationOnly(validateOnly) {}
        ~NativeFlash() override { abort(); }
        bool begin(size_t size) override { return partition && size <= partition->size && esp_ota_begin(partition, size, &handle) == ESP_OK; }
        bool write(const uint8_t *data, size_t size) override { return handle && esp_ota_write(handle, data, size) == ESP_OK; }
        bool finish() override
        {
            const auto value = handle;
            handle = 0;
            return value && esp_ota_end(value) == ESP_OK;
        }
        bool activate(const std::string &version) override
        {
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
            if (validationOnly)
                return true;
#endif
            // Persist recovery intent before the only operation that changes boot selection.
            Preferences preferences;
            if (!preferences.begin(NVS, false) || preferences.putString("pending", version.c_str()) != version.size())
                return false;
            if (esp_ota_set_boot_partition(partition) != ESP_OK)
            {
                preferences.remove("pending");
                return false;
            }
            return true;
        }
        void abort() override
        {
            if (handle)
            {
                esp_ota_abort(handle);
                handle = 0;
            }
        }

    private:
        const esp_partition_t *partition;
        const bool validationOnly;
        esp_ota_handle_t handle = 0;
    };
}
UpdateSystem &UpdateSystem::instance()
{
    static UpdateSystem system;
    return system;
}
UpdateSnapshot UpdateSystem::snapshot() const
{
    TaskMutex::Guard lock(mutex);
    return status;
}
void UpdateSystem::fail(UpdateError error)
{
    TaskMutex::Guard lock(mutex);
    status.state = error == UpdateError::Cancelled ? UpdateState::Cancelled : UpdateState::Error;
    status.error = error;
    status.cancelling = false;
    ++status.revision;
    Log::warn(TAG, "Update stopped: code=%u", unsigned(error));
}
bool UpdateSystem::check() { return request(false); }
bool UpdateSystem::install() { return request(true); }
bool UpdateSystem::request(bool installing)
{
    TaskMutex::Guard lock(mutex);
    if (working || status.state == UpdateState::Success)
        return false;
#if !OTA_ENABLED || !WIFI_ENABLED
    fail(UpdateError::Disabled);
    return false;
#endif
    if (WiFi.status() != WL_CONNECTED)
    {
        fail(UpdateError::Network);
        return false;
    }
    if (pendingBoot())
    {
        fail(UpdateError::BootPending);
        return false;
    }
    if (installing)
    {
        if (!status.release.firmwareId || !status.release.size || UpdateMetadata::compareVersions(status.release.version, FW_VERSION) <= 0)
            return false;
        auto *lua = static_cast<LuaSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_LUA));
        if (lua && lua->isScriptLoaded())
        {
            fail(UpdateError::GameActive);
            return false;
        }
    }
    if (!rollbackEnabled())
    {
        fail(UpdateError::RollbackUnavailable);
        return false;
    }
    if (!updateSlot())
    {
        fail(UpdateError::Incompatible);
        return false;
    }
    if (!UpdateMetadata::versionValid(FW_VERSION))
    {
        fail(UpdateError::Metadata);
        return false;
    }
    cancellation.store(false);
    startedAt = millis();
    installRequested = installing;
    if (!installing)
        status.release = UpdateRelease{};
    status.error = UpdateError::None;
    status.cancelling = false;
    status.downloaded = status.written = 0;
    status.total = installing ? status.release.size : 0;
    status.state = installing ? UpdateState::Downloading : UpdateState::Checking;
    ++status.revision;
    working = true;
    if (xTaskCreatePinnedToCore(worker, "githubOTA", 12288, this, 1, nullptr, 0) != pdPASS)
    {
        working = false;
        fail(UpdateError::Memory);
        return false;
    }
    return true;
}
bool UpdateSystem::stopped() const
{
    return cancellation.load() || uint32_t(millis() - startedAt) >= UpdateConfig::OPERATION_TIMEOUT_MS;
}
void UpdateSystem::cancel()
{
    TaskMutex::Guard lock(mutex);
    if (!working || status.state == UpdateState::Installing || status.state == UpdateState::Success)
        return;
    cancellation.store(true);
    status.cancelling = true;
    ++status.revision;
}
void UpdateSystem::progress(UpdateState state, size_t downloaded, size_t written)
{
    TaskMutex::Guard lock(mutex);
    const bool changed = status.state != state || status.downloaded / 4096 != downloaded / 4096 || status.written / 4096 != written / 4096 || written == status.total;
    status.state = state;
    status.downloaded = downloaded;
    status.written = written;
    if (changed)
        ++status.revision;
}
void UpdateSystem::worker(void *context)
{
    auto &self = *static_cast<UpdateSystem *>(context);
    self.run();
    flashing.store(false);
    {
        TaskMutex::Guard lock(self.mutex);
        self.working = false;
    }
    vTaskDelete(nullptr);
}
void UpdateSystem::run()
{
    // TLS verification needs a real clock; no certificate verification is bypassed.
    if (std::time(nullptr) < 1704067200)
    {
        configTime(0, 0, "pool.ntp.org", "time.cloudflare.com", "time.google.com");
        const auto begin = millis();
        while (std::time(nullptr) < 1704067200 && uint32_t(millis() - begin) < 20000 && !stopped())
            vTaskDelay(pdMS_TO_TICKS(100));
        if (stopped())
        {
            fail(cancellation.load() ? UpdateError::Cancelled : UpdateError::Timeout);
            return;
        }
        if (std::time(nullptr) < 1704067200)
        {
            fail(UpdateError::Clock);
            return;
        }
    }
    UpdateHttp http([this]()
                    { return stopped(); });
    if (!installRequested)
    {
        UpdateRelease release;
        UpdateBuffer buffer;
        size_t size = 0;
        auto error = http.json(apiBase() + "/releases/latest", UpdateConfig::JSON_LIMIT, buffer, size);
        if (error == UpdateError::None)
            error = UpdateMetadata::parseRelease(buffer.get(), size, release);
        buffer.reset();
        if (error == UpdateError::None)
        {
            error = http.json(assetUrl(release.manifestId), UpdateConfig::MANIFEST_LIMIT, buffer, size, true);
            if (error == UpdateError::None)
            {
                const auto *slot = updateSlot();
                error = slot ? UpdateMetadata::parseManifest(buffer.get(), size, release, OTA_HARDWARE_ID, OTA_PARTITION_TABLE_SHA256, slot->size) : UpdateError::Incompatible;
            }
        }
        if (stopped())
            error = cancellation.load() ? UpdateError::Cancelled : UpdateError::Timeout;
        if (error != UpdateError::None)
        {
            fail(error);
            return;
        }
        release.assets.clear();
        release.assets.shrink_to_fit();
        TaskMutex::Guard lock(mutex);
        status.release = std::move(release);
        status.total = status.release.size;
        status.state = UpdateMetadata::compareVersions(status.release.version, FW_VERSION) > 0 ? UpdateState::Available : UpdateState::UpToDate;
        ++status.revision;
        return;
    }
    const auto release = snapshot().release;
    const auto *slot = updateSlot();
    if (!slot || release.size > slot->size)
    {
        fail(UpdateError::Incompatible);
        return;
    }
    if (!http.open(assetUrl(release.firmwareId), true))
    {
        fail(http.error());
        return;
    }
    auto *audio = static_cast<AudioSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    if (audio)
        audio->stop();
    flashing.store(true);
    vTaskDelay(pdMS_TO_TICKS(40)); // Drain existing RMT/I2S activity before erasing flash.
    const auto quietStarted = millis();
    while (audio && !audio->quiescentForFlash() && uint32_t(millis() - quietStarted) < 5000 && !stopped())
        vTaskDelay(pdMS_TO_TICKS(10));
    if (audio && !audio->quiescentForFlash())
    {
        fail(UpdateError::Busy);
        return;
    }
    NativeFlash flash(slot);
    const auto error = UpdateEngine::install(http, flash, release, [this](UpdateState state, size_t downloaded, size_t written)
                                             { progress(state, downloaded, written); }, [this]()
                                             { return stopped(); });
    if (error != UpdateError::None)
    {
        const auto result = stopped() ? (cancellation.load() ? UpdateError::Cancelled : UpdateError::Timeout) : error == UpdateError::Network && http.error() != UpdateError::None ? http.error()
                                                                                                                                                                                   : error;
        fail(result);
        return;
    }
    progress(UpdateState::Success, release.size, release.size);
    Log::info(TAG, "Firmware verified and activated; rebooting");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP.restart();
}
void UpdateSystem::uiHeartbeat() { heartbeat.store(millis()); }
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
UpdateError UpdateSystem::validateInactiveForDiagnostics(UpdateInput &input, const UpdateRelease &release)
{
    UpdateSnapshot original;
    {
        TaskMutex::Guard lock(mutex);
        if (working || pendingBoot())
            return UpdateError::Busy;
        original = status;
        working = true;
        startedAt = millis();
        cancellation.store(false);
    }
    const auto *slot = updateSlot();
    auto *audio = static_cast<AudioSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    if (audio)
        audio->stop();
    flashing.store(true);
    const auto started = millis();
    while (audio && !audio->quiescentForFlash() && uint32_t(millis() - started) < 5000)
        vTaskDelay(pdMS_TO_TICKS(10));
    vTaskDelay(pdMS_TO_TICKS(40));
    UpdateError result = UpdateError::Incompatible;
    if (slot && release.size <= slot->size && (!audio || audio->quiescentForFlash()))
    {
        NativeFlash flash(slot, true);
        result = UpdateEngine::install(input, flash, release, [](UpdateState, size_t, size_t) {}, [this]()
                                       { return stopped(); });
    }
    flashing.store(false);
    {
        TaskMutex::Guard lock(mutex);
        working = false;
        status = original;
        ++status.revision;
    }
    return result;
}
#endif
void UpdateSystem::beginBootVerification()
{
    Preferences preferences;
    std::string expected;
    bool interrupted = false;
    if (preferences.begin(NVS, true))
    {
        expected = preferences.getString("pending", "").c_str();
        interrupted = preferences.getUChar("result", 0) == 3;
        TaskMutex::Guard lock(mutex);
        status.confirmedUpdate = preferences.getUChar("result", 0) == 2 && preferences.getString("last", "") == FW_VERSION;
    }
    if (!pendingBoot() && expected.empty())
    {
        if (interrupted) fail(UpdateError::Interrupted);
        return;
    }
    if (!pendingBoot() && expected != FW_VERSION)
    {
        recordBoot(false);
        fail(UpdateError::Interrupted);
        return;
    }
    if (xTaskCreatePinnedToCore(verifier, "otaVerify", 4096, this, 1, nullptr, 0) != pdPASS)
    {
        if (pendingBoot())
        {
            esp_ota_mark_app_invalid_rollback_and_reboot();
            ESP.restart();
        }
    }
}
void UpdateSystem::verifier(void *context)
{
    auto &self = *static_cast<UpdateSystem *>(context);
    const auto started = millis();
    uint32_t healthySince = 0;
    const EventBits_t required = EVT_BOOT_DONE | EVT_UI_READY | EVT_AUDIO_READY | EVT_LUA_READY;
    while (uint32_t(millis() - started) < 45000)
    {
        const auto now = millis();
        const bool ready = SystemEventBus::systemEvents && (xEventGroupGetBits(SystemEventBus::systemEvents) & required) == required &&
                           self.heartbeat.load() && uint32_t(now - self.heartbeat.load()) < 1000 && heap_caps_get_free_size(MALLOC_CAP_INTERNAL) > 24 * 1024;
        if (!ready)
            healthySince = 0;
        else if (!healthySince)
            healthySince = now;
        else if (uint32_t(now - healthySince) >= 3000)
        {
            if (!pendingBoot() || esp_ota_mark_app_valid_cancel_rollback() == ESP_OK)
            {
                recordBoot(true);
                {
                    TaskMutex::Guard lock(self.mutex);
                    self.status.confirmedUpdate = true;
                    if (self.status.error == UpdateError::BootPending)
                    {
                        self.status.state = UpdateState::Idle;
                        self.status.error = UpdateError::None;
                    }
                    ++self.status.revision;
                }
                Log::info(TAG, "OTA boot confirmed after services and UI health checks");
                vTaskDelete(nullptr);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    recordBoot(false);
    Log::error(TAG, "OTA boot health check failed; rolling back");
    if (pendingBoot())
    {
        esp_ota_mark_app_invalid_rollback_and_reboot();
        ESP.restart();
    }
    vTaskDelete(nullptr);
}
