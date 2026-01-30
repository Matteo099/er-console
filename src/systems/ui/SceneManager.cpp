#include "systems/ui/SceneManager.h"

#include "scenes/menu/MainMenuScene.h"
#include "scenes/about/AboutScene.h"
#include "scenes/gamelist/GameListScene.h"
#include "scenes/settings/SettingsScene.h"

static lv_scr_load_anim_t toLvgl(SceneTransition t)
{
    switch (t)
    {
    case SceneTransition::Fade:
        return LV_SCR_LOAD_ANIM_FADE_ON;

    case SceneTransition::SlideLeft:
        return LV_SCR_LOAD_ANIM_MOVE_LEFT;

    case SceneTransition::SlideRight:
        return LV_SCR_LOAD_ANIM_MOVE_RIGHT;

    case SceneTransition::None:
    default:
        return LV_SCR_LOAD_ANIM_NONE;
    }
}

void SceneManager::init(void *params)
{
    current = nullptr;
}

void SceneManager::changeScene(Scene *next)
{
    if (!next)
        return;

    if (current)
    {
        current->onExit();
        current->getView().destroy();
        delete current;
    }

    current = next;

    current->getView().create();
    current->onEnter();

    lv_scr_load_anim(
        current->getView().getScreen(),
        toLvgl(current->getTransition()),
        300,
        0,
        false);
}

void SceneManager::update()
{
    if (!current)
        return;

    current->onUpdate();
    current->getView().update();

    SceneRequest req = current->getRequest();

    if (!req.isNone())
    {
        switch (req.change)
        {
        case SceneChange::GoToMainMenu:
            changeScene(new MainMenuScene());
            break;

        case SceneChange::GoToGameList:
            changeScene(new GameListScene());
            break;

        case SceneChange::GoToSettings:
            changeScene(new SettingsScene());
            break;

        case SceneChange::GoToAbout:
            changeScene(new AboutScene());
            break;

        default:
            break;
        }
    }
}
