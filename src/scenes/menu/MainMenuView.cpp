#include "MainMenuView.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"
#include "config/Console.h"

void MainMenuView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen, 44, 0, false);
    brandImg = BrandHeader::create(layout.top);

    menuContainer = Container::create(layout.body, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_MID);
    Container::setFlex(menuContainer, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(menuContainer, 6, 0);
    Label::create(menuContainer, "La prossima avventura", &lv_font_montserrat_20);
    auto *subtitle = Label::create(menuContainer,
#if WIFI_ENABLED
                                   "Scegli una storia. Mettiti alla prova.",
#else
                                   "Uso locale. La sfida inizia qui.",
#endif
                                   &lv_font_montserrat_12);
    UITheme::muted(subtitle);

    btnNewGame = Button::create(menuContainer, "Gioca", onButtonClicked, this,
                                LV_PCT(100), 62, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_PLAY);
    lv_obj_set_style_text_font(lv_obj_get_child(btnNewGame, 1), &lv_font_montserrat_20, 0);
    auto *arrow = Label::create(btnNewGame, LV_SYMBOL_RIGHT, &lv_font_montserrat_14);
    UITheme::accent(arrow);

    auto *row = Container::create(menuContainer, LV_PCT(100), 52);
    Container::setFlex(row, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(row, 8, 0);
    btnSettings = Button::create(row, "Impostazioni", onButtonClicked, this, 0, 48,
                                 LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_SETTINGS);
    btnAbout = Button::create(row, "Info", onButtonClicked, this, 0, 48,
                              LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_LIST);
    lv_obj_set_flex_grow(btnSettings, 2);
    lv_obj_set_flex_grow(btnAbout, 1);
    UITheme::secondary(btnSettings);
    UITheme::secondary(btnAbout);
}

void MainMenuView::setStorageState(StorageState state)
{
    if (!btnNewGame) return;
    const bool unavailable = state == StorageState::Unavailable || state == StorageState::Error;
    lv_obj_set_state(btnNewGame, LV_STATE_DISABLED, unavailable);
}

void MainMenuView::update() {}
void MainMenuView::destroy()
{
    screen = brandImg = menuContainer = nullptr;
    btnNewGame = btnSettings = btnAbout = nullptr;
}
void MainMenuView::onButtonClicked(lv_event_t *e)
{
    auto *view = static_cast<MainMenuView *>(lv_event_get_user_data(e));
    if (!view) return;
    auto *target = static_cast<lv_obj_t *>(lv_event_get_target(e));
    if (target == view->btnNewGame) view->emitEvent(ButtonClickedEvent("new_game"));
    else if (target == view->btnSettings) view->emitEvent(ButtonClickedEvent("settings"));
    else if (target == view->btnAbout) view->emitEvent(ButtonClickedEvent("about"));
}
