#pragma once

#include <stdint.h>
#include "systems/ui/View.h"
#include "GameAction.h"

class GameView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };
    GameAction consumeAction();

private:
    lv_obj_t *screen = nullptr;

    lv_obj_t* brandImg = nullptr;
    lv_obj_t* container = nullptr;
    lv_obj_t* textLabel = nullptr;
    lv_obj_t* btnBack = nullptr;
    GameAction pendingAction = GameAction::None;

    static void onBackClicked(lv_event_t* e);
};
