#include "GameListScene.h"
#include "log/Logger.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "scenes/menu/MainMenuScene.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"

void GameListScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering GameListScene");
    loadGames();
    view.setGames(games);
}

void GameListScene::onUpdate()
{
    if (view.hasSelectionChanged())
    {
        selectedIndex = view.getSelectedIndex();
    }

    switch (view.consumeAction())
    {
    case GameListAction::Back:
        request = SceneRequest(SceneChange::GoToMainMenu);
        break;

    case GameListAction::StartGame:
        // TODO: &games[selectedIndex].id - is it correct?
        request = SceneRequest(SceneChange::GoToGame, &games[selectedIndex].id);
        break;

    default:
        break;
    }
}

void GameListScene::onExit()
{
}


SceneRequest GameListScene::getRequest() const
{
    return request;
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