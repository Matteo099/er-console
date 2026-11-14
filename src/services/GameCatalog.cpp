#include "GameCatalog.h"
#include "GameMetadata.h"
#include "core/TaskMutex.h"
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include "config/Assets.h"
#include "log/Logger.h"
#include "systems/update/SdResources.h"
#include <ArduinoJson.h>

namespace {
TaskMutex catalogMutex;
GameCatalog::Snapshot cachedCatalog;
}  // namespace

GameCatalog::Snapshot GameCatalog::readGames() {
    Snapshot result;
    result.initialized = true;
    if (!SdResources::current()) {
        result.error = Text::SdResourcesMissing;
        return result;
    }
    StorageSystem* storageSystem = static_cast<StorageSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));

    if (!storageSystem || !storageSystem->isReady()) {
        Log::error(Log::SCENE_TAG, "Storage system is not ready");
        result.error = Text::StorageUnavailable;
        return result;
    }

    std::vector<StorageEntry> entries;
    if (!storageSystem->listDirectory(GAMES_DIR, entries, false)) {
        Log::error(Log::SCENE_TAG, "Failed to list games directory");
        result.error = Text::CatalogReadError;
        return result;
    }

    for (const auto& entry : entries) {
        if (!entry.isDirectory)
            continue;

        std::string configPath = std::string(GAMES_DIR) + "/" + entry.name + "/config.json";
        std::string jsonText;
        if (!storageSystem->readTextFile(configPath.c_str(), jsonText, true)) {
            Log::warn(Log::SCENE_TAG, "Missing config.json for game: %s", entry.name.c_str());
            result.error = Text::CatalogFilesError;
            continue;
        }

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, jsonText);
        if (err) {
            Log::error(Log::SCENE_TAG, "JSON parse error in %s: %s", entry.name.c_str(),
                       err.c_str());
            result.error = Text::CatalogConfigError;
            continue;
        }

        GameInfo info;
        info.id = entry.name;
        GameMetadata::read(doc.as<JsonObjectConst>(), info);
        info.inputConfig = InputConfig();
        JsonVariantConst inputs = doc["inputs"];
        info.inputConfig = InputConfig::parse(inputs);
        result.games.push_back(info);
        Log::debug(Log::SCENE_TAG, "Loaded game: %s (%s)", info.id.c_str(), info.title.c_str());
    }

    return result;
}

bool GameCatalog::snapshot(uint32_t seenVersion, Snapshot& result) {
    TaskMutex::Guard lock(catalogMutex);
    if (seenVersion == cachedCatalog.version)
        return false;
    result = cachedCatalog;
    return true;
}

void GameCatalog::request(bool force) {
    {
        TaskMutex::Guard lock(catalogMutex);
        if (cachedCatalog.loading || (cachedCatalog.initialized && !force))
            return;
        cachedCatalog.loading = true;
        ++cachedCatalog.version;
    }
    if (xTaskCreatePinnedToCore(loadTask, "gameCatalog", 8192, nullptr, 1, nullptr, 0) != pdPASS) {
        TaskMutex::Guard lock(catalogMutex);
        cachedCatalog.loading = false;
        cachedCatalog.initialized = true;
        cachedCatalog.error = Text::MemoryError;
        ++cachedCatalog.version;
    }
}

void GameCatalog::loadTask(void*) {
    auto result = readGames();
    if (result.games.empty() && result.error != Text::None) {
        vTaskDelay(pdMS_TO_TICKS(100));
        result = readGames();  // One bounded retry; the UI remains available.
    }
    {
        TaskMutex::Guard lock(catalogMutex);
        if (result.error != Text::None && result.error != Text::SdResourcesMissing &&
            result.games.empty())
            result.games.swap(cachedCatalog.games);
        result.version = cachedCatalog.version + 1;
        cachedCatalog = std::move(result);
    }
    vTaskDelete(nullptr);
}
