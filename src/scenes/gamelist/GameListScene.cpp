#include "GameListScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "scenes/menu/MainMenuScene.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"

void GameListScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering GameListScene");
    view.setEventHandler(this);
    loadGames();
    view.setGames(games);
    view.create();
}

void GameListScene::onUpdate()
{
    if (view.hasSelectionChanged())
    {
        selectedIndex = view.getSelectedIndex();
    }
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


bool GameListScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToMainMenu);
        return true;
    }

    if (clicked.buttonId == "start_game")
    {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(games.size()))
        {
            request = SceneRequest(SceneChange::GoToGame, &games[selectedIndex].id);
        }
        return true;
    }

    return false;
}


void GameListScene::loadGames()
{
    // TODO: replace with SD scan + manifest parsing
    games = {
        {
            "lab01",
            "The Abandoned Lab",
            "A mysterious laboratory sealed after a failed experiment.",
            3,
            45
        },
        {
            "vault",
            "The Forgotten Vault",
            "Crack the security system before the vault locks forever.",
            4,
            60
        }
    };
}