#include "GameListScene.h"
#include "log/Logger.h"
#include "services/GameCatalog.h"
#include "systems/ui/UIEvents.h"

void GameListScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering GameListScene");
    view.setEventHandler(this);
    GameCatalog::request();
    refreshCatalog();
    view.create();
}

void GameListScene::refreshCatalog()
{
    GameCatalog::Snapshot catalog;
    if (!GameCatalog::snapshot(catalogVersion, catalog)) return;
    catalogVersion = catalog.version;
    view.setCatalog(catalog.games, catalog.error, catalog.loading);
}

void GameListScene::onUpdate() { refreshCatalog(); view.update(); }
void GameListScene::onExit() { view.destroy(); }
SceneRequest GameListScene::getRequest() const { return request; }

bool GameListScene::handleUIEvent(const UIEvent &event)
{
    if (event.getType() != ButtonClickedEvent::TYPE) return false;
    const auto &clicked = static_cast<const ButtonClickedEvent &>(event);
    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToMainMenu, SceneTransition::SlideRight);
        return true;
    }
    if (clicked.buttonId == "refresh_games")
    {
        GameCatalog::request(true);
        return true;
    }
    if (clicked.buttonId == "start_game")
    {
        if (auto *game = view.getCurrentGame())
        {
            // The cache may publish a refresh before SceneManager consumes this request.
            selectedGame = *game;
            request = SceneRequest(SceneChange::GoToGame, SceneTransition::SlideLeft, &selectedGame);
        }
        else GameCatalog::request(true);
        return true;
    }
    return false;
}
