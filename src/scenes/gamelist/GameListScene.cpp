#include "GameListScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "scenes/menu/MainMenuScene.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"
#include "systems/storage/StorageSystem.h"
#include "config/Assets.h"
#include <ArduinoJson.h>

void GameListScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering GameListScene");
    view.setEventHandler(this);
    loadGames();
    view.create();
    view.setGames(games);
}

void GameListScene::onUpdate()
{
    view.update();
}

void GameListScene::onExit()
{
    view.destroy();
}

SceneRequest GameListScene::getRequest() const
{
    return request;
}

bool GameListScene::handleUIEvent(const UIEvent &event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto &clicked = static_cast<const ButtonClickedEvent &>(event);
    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToMainMenu, SceneTransition::SlideRight);
        return true;
    }

    if (clicked.buttonId == "start_game")
    {
        if (GameInfo *game =
                view.getCurrentGame())
        {
            request = SceneRequest(
                SceneChange::GoToGame,
                SceneTransition::SlideLeft,
                game);
        }

        return true;
    }

    return false;
}

void GameListScene::loadGames()
{
    games.clear();
    StorageSystem *storageSystem = static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));

    if (!storageSystem || !storageSystem->isReady())
        return;

    std::vector<StorageEntry> entries;
    if (!storageSystem->listDirectory(GAMES_DIR, entries, false))
    {
        Log::error(Log::SCENE_TAG, "Failed to list games directory");
        return;
    }

    for (const auto &entry : entries)
    {
        if (!entry.isDirectory)
            continue;

        std::string configPath = std::string(GAMES_DIR) + "/" + entry.name + "/config.json";
        std::string jsonText;
        if (!storageSystem->readTextFile(configPath.c_str(), jsonText))
        {
            Log::warn(Log::SCENE_TAG, "Missing config.json for game: %s", entry.name.c_str());
            continue;
        }

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, jsonText);
        if (err)
        {
            Log::error(Log::SCENE_TAG, "JSON parse error in %s: %s", entry.name.c_str(), err.c_str());
            continue;
        }

        GameInfo info;
        info.id = entry.name;
        info.title = doc["name"].is<const char *>() ? doc["name"].as<const char *>() : entry.name;
        info.description = doc["description"].as<const char *>() ? doc["description"].as<const char *>() : "";
        info.author = doc["author"].as<const char *>() ? doc["author"].as<const char *>() : "";
        info.difficulty = doc["difficulty"] | 1;
        info.durationMin = doc["durationMin"] | 0;
        games.push_back(info);
        Log::debug(Log::SCENE_TAG, "Loaded game: %s (%s)", info.id.c_str(), info.title.c_str());
    }
}