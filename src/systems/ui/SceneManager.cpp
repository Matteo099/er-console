#include "systems/ui/SceneManager.h"

#include <lvgl.h>

void SceneManager::changeScene(Scene* next)
{
    if (current) {
        current->onExit();
        delete current;
    }

    current = next;
    current->onEnter();

    lv_scr_load_anim(
        current->getView().getScreen(),
        LV_SCR_LOAD_ANIM_MOVE_LEFT,
        300,
        0,
        false
    );
}


void SceneManager::update()
{
    if (next)
    {
        if (current)
            current->onExit();
        current = next;
        next = nullptr;
        current->onEnter();
    }

    if (current)
        current->onUpdate();
}