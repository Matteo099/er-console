#include "MainMenuView.h"
#include "log/Logger.h"
#include <cstring>

#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Screen.h"

void MainMenuView::create()
{
    Log::debug(Log::UI_TAG, "Creating MainMenuView");
    screen = Screen::create();

    brandImg = BrandHeader::create(screen);

    menuContainer = Container::create(screen, 220, 140, LV_ALIGN_CENTER, 0, 10);
    Container::setFlex(menuContainer);

    btnNewGame = Button::create(menuContainer, "New Game", onButtonClicked, this);
    btnSettings = Button::create(menuContainer, "Settings", onButtonClicked, this);
    btnAbout = Button::create(menuContainer, "About", onButtonClicked, this);
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
        view->pendingAction = MainMenuAction::NewGame;
    else if (target == view->btnSettings)
        view->pendingAction = MainMenuAction::Settings;
    else if (target == view->btnAbout)
        view->pendingAction = MainMenuAction::About;
}

MainMenuAction MainMenuView::consumeAction()
{
    MainMenuAction a = pendingAction;
    pendingAction = MainMenuAction::None;
    return a;
}
