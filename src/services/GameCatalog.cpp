#include "GameCatalog.h"
#include "core/TaskMutex.h"
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include "config/Assets.h"
#include "log/Logger.h"
#include <ArduinoJson.h>

namespace
{
TaskMutex catalogMutex;
GameCatalog::Snapshot cachedCatalog;
}

static void loadInputConfig(JsonObject inputs, InputConfig &config)
{
    if (inputs.isNull())
        return;

    // POWER
    if (auto power = inputs["power"].as<JsonObject>())
    {
        config.power.enabled = power["enabled"] | config.power.enabled;
        config.power.address = power["address"] | config.power.address;
    }

    // RFID
    if (auto rfid = inputs["rfid"].as<JsonObject>())
    {
        config.rfid.enabled = rfid["enabled"] | config.rfid.enabled;
        config.rfid.ssPin = rfid["ssPin"] | config.rfid.ssPin;
        config.rfid.rstPin = rfid["rstPin"] | config.rfid.rstPin;
    }

    // OLED
    if (auto oled = inputs["oled"].as<JsonObject>())
    {
        config.oled.enabled = oled["enabled"] | config.oled.enabled;
        config.oled.width = oled["width"] | config.oled.width;
        config.oled.height = oled["height"] | config.oled.height;
        config.oled.address = oled["address"] | config.oled.address;
    }

    // CHANNELS
    if (auto ch = inputs["channels"].as<JsonObject>())
    {
        config.channels.enabled = ch["enabled"] | config.channels.enabled;
        config.channels.mcp1Enabled = ch["mcp1Enabled"] | config.channels.mcp1Enabled;
        config.channels.mcp2Enabled = ch["mcp2Enabled"] | config.channels.mcp2Enabled;
        config.channels.muxEnabled = ch["muxEnabled"] | config.channels.muxEnabled;
        config.channels.mcp1Base = ch["mcp1Base"] | config.channels.mcp1Base;
        config.channels.mcp1Address = ch["mcp1Address"] | config.channels.mcp1Address;
        config.channels.mcp2Base = ch["mcp2Base"] | config.channels.mcp2Base;
        config.channels.mcp2Address = ch["mcp2Address"] | config.channels.mcp2Address;
        config.channels.muxBase = ch["muxBase"] | config.channels.muxBase;
        config.channels.muxCsPin = ch["muxCsPin"] | config.channels.muxCsPin;
        config.channels.muxS0Pin = ch["muxS0Pin"] | config.channels.muxS0Pin;
        config.channels.muxS1Pin = ch["muxS1Pin"] | config.channels.muxS1Pin;
        config.channels.muxS2Pin = ch["muxS2Pin"] | config.channels.muxS2Pin;
        config.channels.muxS3Pin = ch["muxS3Pin"] | config.channels.muxS3Pin;
    }

    // RGB
    if (auto rgb = inputs["rgb"].as<JsonObject>())
    {
        config.rgb.enabled = rgb["enabled"] | config.rgb.enabled;
        config.rgb.usePCA9685 = rgb["usePCA9685"] | config.rgb.usePCA9685;
        config.rgb.address = rgb["address"] | config.rgb.address;
    }

    // SEVEN SEGMENT
    if (auto seg = inputs["sevenSegment"].as<JsonObject>())
    {
        config.sevenSegment.enabled = seg["enabled"] | config.sevenSegment.enabled;
        config.sevenSegment.csPin = seg["csPin"] | config.sevenSegment.csPin;
        config.sevenSegment.digits = seg["digits"] | config.sevenSegment.digits;
    }

    // RELAYS
    if (auto relays = inputs["relays"].as<JsonObject>())
    {
        config.relays.enabled = relays["enabled"] | config.relays.enabled;
    }
}


GameCatalog::Snapshot GameCatalog::readGames()
{

    Snapshot result;
    result.initialized = true;
    StorageSystem *storageSystem = static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));

    if (!storageSystem || !storageSystem->isReady())
    {
        Log::error(Log::SCENE_TAG, "Storage system is not ready");
        result.error = Text::StorageUnavailable;
        return result;
    }

    std::vector<StorageEntry> entries;
    if (!storageSystem->listDirectory(GAMES_DIR, entries, false))
    {
        Log::error(Log::SCENE_TAG, "Failed to list games directory");
        result.error = Text::CatalogReadError;
        return result;
    }

    for (const auto &entry : entries)
    {
        if (!entry.isDirectory)
            continue;

        std::string configPath = std::string(GAMES_DIR) + "/" + entry.name + "/config.json";
        std::string jsonText;
        if (!storageSystem->readTextFile(configPath.c_str(), jsonText, true))
        {
            Log::warn(Log::SCENE_TAG, "Missing config.json for game: %s", entry.name.c_str());
            result.error = Text::CatalogFilesError;
            continue;
        }

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, jsonText);
        if (err)
        {
            Log::error(Log::SCENE_TAG, "JSON parse error in %s: %s", entry.name.c_str(), err.c_str());
            result.error = Text::CatalogConfigError;
            continue;
        }

        GameInfo info;
        info.id = entry.name;
        info.title = doc["name"].is<const char *>() ? doc["name"].as<const char *>() : entry.name;
        info.description = doc["description"].as<const char *>() ? doc["description"].as<const char *>() : "";
        info.author = doc["author"].as<const char *>() ? doc["author"].as<const char *>() : "";
        info.difficulty = doc["difficulty"] | 1;
        info.durationMin = doc["durationMin"] | 0;
        info.inputConfig = InputConfig();
        JsonObject inputs = doc["inputs"];
        loadInputConfig(inputs, info.inputConfig);
        result.games.push_back(info);
        Log::debug(Log::SCENE_TAG, "Loaded game: %s (%s)", info.id.c_str(), info.title.c_str());
    }

    return result;
}

bool GameCatalog::snapshot(uint32_t seenVersion, Snapshot &result)
{
    TaskMutex::Guard lock(catalogMutex);
    if (seenVersion == cachedCatalog.version) return false;
    result = cachedCatalog;
    return true;
}

void GameCatalog::request(bool force)
{
    {
        TaskMutex::Guard lock(catalogMutex);
        if (cachedCatalog.loading || (cachedCatalog.initialized && !force)) return;
        cachedCatalog.loading = true;
        ++cachedCatalog.version;
    }
    if (xTaskCreatePinnedToCore(loadTask, "gameCatalog", 8192, nullptr, 1, nullptr, 0) != pdPASS)
    {
        TaskMutex::Guard lock(catalogMutex);
        cachedCatalog.loading = false;
        cachedCatalog.initialized = true;
        cachedCatalog.error = Text::MemoryError;
        ++cachedCatalog.version;
    }
}

void GameCatalog::loadTask(void *)
{
    auto result = readGames();
    if (result.games.empty() && result.error != Text::None)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        result = readGames(); // One bounded retry; the UI remains available.
    }
    {
        TaskMutex::Guard lock(catalogMutex);
        if (result.error != Text::None && result.games.empty()) result.games.swap(cachedCatalog.games);
        result.version = cachedCatalog.version + 1;
        cachedCatalog = std::move(result);
    }
    vTaskDelete(nullptr);
}
