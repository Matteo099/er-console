#include "GameListView.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/GameCard.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"
#include "systems/ui/components/ScrollList.h"

void GameListView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen);

    header = BrandHeader::create(layout.top, 100, "Le tue avventure");

    cardContainer = lv_obj_create(layout.body);
    lv_obj_remove_style_all(cardContainer);
    lv_obj_set_size(cardContainer, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(cardContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(cardContainer, LV_ALIGN_TOP_MID, 0, 0);

    // FOOTER
    auto footerRow = lv_obj_create(layout.footer);
    lv_obj_remove_style_all(footerRow);
    lv_obj_set_size(footerRow, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(footerRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footerRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(footerRow, 6, 0);

    // ICON ONLY BUTTONS
    btnBack = Button::create(footerRow, nullptr, onBackClicked, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_HOME);
    btnPrev = Button::create(footerRow, nullptr, onPrevClicked, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_LEFT);
    UITheme::secondary(btnBack);
    UITheme::secondary(btnPrev);

    pageLabel = lv_label_create(footerRow);
    lv_obj_set_style_text_font(pageLabel, &lv_font_montserrat_12, 0);

    btnNext = Button::create(footerRow, nullptr, onNextClicked, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_RIGHT);
    UITheme::secondary(btnNext);
    btnStart = Button::create(footerRow, "Avvia", onStartClicked, this, 98, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_PLAY);

    rebuildCurrentCard();
}

void GameListView::setGames(
    const std::vector<GameInfo> &g)
{
    games = g;

    currentIndex = 0;

    rebuildCurrentCard();
}

void GameListView::rebuildCurrentCard()
{
    if (!cardContainer)
        return;

    lv_obj_clean(cardContainer);

    if (games.empty())
    {
        auto *title = Label::create(cardContainer, "Nessuna avventura", &lv_font_montserrat_18,
                                    LV_TEXT_ALIGN_CENTER, LV_PCT(100));
        lv_obj_set_style_pad_top(title, 20, 0);
        auto *help = Label::create(cardContainer, "Gli scenari sulla microSD appariranno qui.",
                                   &lv_font_montserrat_12, LV_TEXT_ALIGN_CENTER, LV_PCT(100));
        lv_obj_align_to(help, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
        UITheme::muted(help);
        lv_label_set_text(pageLabel, "0 / 0");
        lv_obj_add_state(btnStart, LV_STATE_DISABLED);
        lv_obj_add_state(btnPrev, LV_STATE_DISABLED);
        lv_obj_add_state(btnNext, LV_STATE_DISABLED);
        return;
    }
    lv_obj_remove_state(btnStart, LV_STATE_DISABLED);
    lv_obj_set_state(btnPrev, LV_STATE_DISABLED, games.size() < 2);
    lv_obj_set_state(btnNext, LV_STATE_DISABLED, games.size() < 2);
    lv_obj_scroll_to_y(lv_obj_get_parent(cardContainer), 0, LV_ANIM_OFF);

    const auto &game = games[currentIndex];

    GameCard::GameData data{game.title, game.description, game.author, game.difficulty, game.durationMin};
    GameCard::create(cardContainer, data);
    char pageText[32];
    snprintf(pageText, sizeof(pageText), "%u / %u", static_cast<unsigned>(currentIndex + 1), static_cast<unsigned>(games.size()));
    lv_label_set_text(pageLabel, pageText);
}

void GameListView::update()
{
}

void GameListView::onPrevClicked(lv_event_t *e)
{
    auto *view =
        static_cast<GameListView *>(
            lv_event_get_user_data(e));

    if (!view || view->games.empty())
        return;

    if (view->currentIndex == 0)
        view->currentIndex =
            view->games.size() - 1;
    else
        --view->currentIndex;

    view->rebuildCurrentCard();
}

GameInfo *GameListView::getCurrentGame()
{
    if (games.empty())
        return nullptr;

    return &games[currentIndex];
}

void GameListView::onNextClicked(lv_event_t *e)
{
    auto *view =
        static_cast<GameListView *>(
            lv_event_get_user_data(e));

    if (!view || view->games.empty())
        return;

    view->currentIndex =
        (view->currentIndex + 1) % view->games.size();

    view->rebuildCurrentCard();
}

void GameListView::onStartClicked(lv_event_t *e)
{
    auto *view = static_cast<GameListView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("start_game"));
}

void GameListView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<GameListView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("back"));
}

void GameListView::destroy()
{
    screen = nullptr;

    // Nullify remaining pointers
    header = nullptr;
    cardContainer = nullptr;

    btnBack = nullptr;
    btnPrev = nullptr;
    btnNext = nullptr;
    btnStart = nullptr;

    pageLabel = nullptr;

    // Clear dynamic object tracking
    pendingAction = GameListAction::None;
    currentIndex = 0;
}
