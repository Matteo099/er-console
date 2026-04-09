#pragma once

#include <stdint.h>
#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"
#include "AboutAction.h"

class AboutView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };

private:
    lv_obj_t *screen = nullptr;

    lv_obj_t* brandImg = nullptr;
    lv_obj_t* container = nullptr;
    lv_obj_t* textLabel = nullptr;
    lv_obj_t* btnBack = nullptr;
    AboutAction pendingAction = AboutAction::None;

    static void onBackClicked(lv_event_t* e);
};
