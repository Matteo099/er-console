#include "AboutView.h"
#include "config/Environment.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"

void AboutView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen);
    brandImg = BrandHeader::create(layout.top, 100, "Informazioni");
    container = Container::create(layout.body, LV_PCT(100), LV_SIZE_CONTENT, LV_ALIGN_TOP_MID);
    Container::setFlex(container, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    UITheme::card(container);
    lv_obj_set_style_pad_row(container, 8, 0);
    Label::create(container, CONSOLE_TITLE, &lv_font_montserrat_20);
    textLabel = Label::create(container, "Scenari interattivi, tutti da vivere.", &lv_font_montserrat_12,
                              LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(textLabel);
    Label::create(container, "Firmware " FW_VERSION, &lv_font_montserrat_14);
    auto *build = Label::create(container, "Build " FW_BUILD_DATE "\nRevisione " FW_GIT_HASH,
                                &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(build);
    btnBack = BackButton::create(layout.footer, onBackClicked, this);
}
void AboutView::update() {}
void AboutView::destroy() { screen = brandImg = container = textLabel = btnBack = nullptr; }
void AboutView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<AboutView *>(lv_event_get_user_data(e));
    if (view) view->emitEvent(ButtonClickedEvent("back"));
}
