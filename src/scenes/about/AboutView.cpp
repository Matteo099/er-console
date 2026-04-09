#include "AboutView.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/Screen.h"

void AboutView::create()
{
    Log::debug(Log::UI_TAG, "Creating AboutView");

    screen = Screen::create();
    brandImg = BrandHeader::create(screen);

    container = Container::create(screen, 220, 120, LV_ALIGN_CENTER, 0, 10);
    Container::setFlex(container);

    textLabel = Label::create(
        container,
        "EscapeOS\n\n"
        "Open escape-room engine\n"
        "for ESP32-based consoles.\n\n"
        "© 2026",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        220);
    Label::setTextColor(textLabel, lv_color_hex(0xCCCCCC));

    btnBack = BackButton::create(screen, onBackClicked, this);
}

void AboutView::update()
{
    // Static screen — nothing animated for now
}

void AboutView::destroy()
{
    // Clean up tracker (for any dynamic objects)
    getObjectTracker().cleanup();

    // Delete root (cascades to all children)
    UICleanup::deleteRoot(screen);

    // Nullify remaining pointers
    brandImg = nullptr;
    container = nullptr;
    textLabel = nullptr;
    btnBack = nullptr;

    pendingAction = AboutAction::None;
}

void AboutView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<AboutView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("back"));
}
