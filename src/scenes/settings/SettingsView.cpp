#include "SettingsView.h"
#include "log/Logger.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/Screen.h"

void SettingsView::create()
{
    Log::debug(Log::UI_TAG, "Creating SettingsView");

    screen = Screen::create();
    brandImg = BrandHeader::create(screen);

    container = Container::create(screen, 220, 120, LV_ALIGN_CENTER, 0, 10);
    Container::setFlex(container);

    btnSound = Button::create(container, "Sound: OFF", onSoundClicked, this, 200, 42);
    lblSound = lv_obj_get_child(btnSound, 0);

    btnBack = BackButton::create(screen, onBackClicked, this);
}

void SettingsView::update()
{
    // no animations yet
}

void SettingsView::destroy()
{
    // Clean up tracker (for any dynamic objects)
    getObjectTracker().cleanup();

    // Delete root (cascades to all children)
    UICleanup::deleteRoot(screen);

    // Nullify remaining pointers
    brandImg = nullptr;
    container = nullptr;
    btnSound = nullptr;
    lblSound = nullptr;
    btnBack = nullptr;

    pendingAction = SettingsAction::None;
}

void SettingsView::setSoundEnabled(bool enabled)
{
    if (!lblSound)
        return;

    lv_label_set_text(lblSound, enabled ? "Sound: ON" : "Sound: OFF");
}

void SettingsView::onSoundClicked(lv_event_t* e)
{
    auto* view = static_cast<SettingsView*>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->pendingAction = SettingsAction::ToggleSound;
}

void SettingsView::onBackClicked(lv_event_t* e)
{
    auto* view = static_cast<SettingsView*>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->pendingAction = SettingsAction::Back;
}

SettingsAction SettingsView::consumeAction()
{
    SettingsAction a = pendingAction;
    pendingAction = SettingsAction::None;
    return a;
}
