#include "MainMenuView.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"

void MainMenuView::create()
{
    Log::debug(Log::UI_TAG, "Creating MainMenuView");
    screen = Screen::create();

    auto layout = SceneLayout::create(screen, 70, 0, false);

    brandImg = BrandHeader::create(layout.top);

    menuContainer = Container::create(layout.body, 220, 140, LV_ALIGN_TOP_MID, 0, 0);
    Container::setFlex(menuContainer);

    btnNewGame = Button::create(menuContainer, "New Game", onButtonClicked, this, 200, 42, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_PLAY);
    btnSettings = Button::create(menuContainer, "Settings", onButtonClicked, this, 200, 42, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_SETTINGS);
    btnAbout = Button::create(menuContainer, "About", onButtonClicked, this, 200, 42, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_FILE);
}

void MainMenuView::update()
{
    // Nothing animated yet (future: focus animations, idle effects)
}

void MainMenuView::destroy()
{
    // Clean up tracker (for any dynamic objects)
    getObjectTracker().cleanup();

    // Delete root (cascades to all children)
    UICleanup::deleteRoot(screen);

    // Nullify remaining pointers
    brandImg = nullptr;
    menuContainer = nullptr;
    btnNewGame = btnSettings = btnAbout = nullptr;
}

void MainMenuView::onButtonClicked(lv_event_t *e)
{
    auto *view = static_cast<MainMenuView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    lv_obj_t* target = static_cast<lv_obj_t *>(lv_event_get_target(e));
    if (target == view->btnNewGame)
        view->emitEvent(ButtonClickedEvent("new_game"));
    else if (target == view->btnSettings)
        view->emitEvent(ButtonClickedEvent("settings"));
    else if (target == view->btnAbout)
        view->emitEvent(ButtonClickedEvent("about"));
}
