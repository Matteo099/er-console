#include "AboutView.h"
#include "log/Logger.h"
#include "config/Environment.h"

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

    container = Container::create(screen, 240, 140, LV_ALIGN_CENTER, 0, 10);
    Container::setFlex(container);

    // Info icon
    lv_obj_t *iconLabel = Label::create(
        container,
        LV_SYMBOL_FILE,
        nullptr,
        LV_TEXT_ALIGN_CENTER,
        240);
    lv_obj_set_style_text_font(iconLabel, &lv_font_montserrat_30, 0);
    Label::setTextColor(iconLabel, lv_color_hex(0x64B5F6));

    textLabel = Label::create(
        container,
        "EscapeOS\n\n"
        "Portable escape-room platform\n\n"
        "Version " FW_VERSION "\n"
        "Build: " FW_BUILD_DATE "\n"
        "Commit: " FW_GIT_HASH "\n\n",
        &lv_font_montserrat_14,
        LV_TEXT_ALIGN_CENTER,
        240);
    if (FW_DEV_MODE == 1)
    {
        Label::setTextColor(textLabel, lv_color_hex(0xFF5555));
    }
    else
    {
        Label::setTextColor(textLabel, lv_color_hex(0xCCCCCC));
    }

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
