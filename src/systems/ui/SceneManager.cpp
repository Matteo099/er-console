#include "systems/ui/SceneManager.h"
#include "scenes/menu/MainMenuScene.h"
#include <lvgl.h>

static lv_scr_load_anim_t toLvgl(SceneTransition t)
{
    switch (t)
    {
    case SceneTransition::Fade:
        return LV_SCR_LOAD_ANIM_FADE_ON;

    case SceneTransition::SlideLeft:
        return LV_SCR_LOAD_ANIM_MOVE_LEFT;

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

    switch (current->getRequest())
    {
    case SceneRequest::GoToMainMenu:
        changeScene(new MainMenuScene());
        break;

    default:
        break;
    }
}
