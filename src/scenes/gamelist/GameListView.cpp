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
    Log::debug(Log::UI_TAG, "Creating GameListView");
    screen = Screen::create();
    auto layout = SceneLayout::create(screen, 70);
    header = BrandHeader::create(layout.top);
    list = ScrollList::create(layout.body, 220, 180, LV_ALIGN_TOP_MID, 0, 0);
    btnBack = BackButton::create(layout.footer, onBackClicked, this);
    rebuildCards();
}

void GameListView::setGames(const std::vector<GameInfo>& g)
{
    games = g;
    rebuildCards();
}

void GameListView::rebuildCards()
{
    if (!list)
        return;

    lv_obj_clean(list);

    cards.clear();
    cardUserData.clear();
    cardUserData.reserve(games.size());
    selectedIndex = -1;
    selectionChanged = false;

    for (size_t i = 0; i < games.size(); ++i)
    {
        const auto& game = games[i];

        GameCard::GameData cardData{
            game.title,
            game.description,
            game.difficulty,
            game.durationMin
        };

        cardUserData.push_back({ this, static_cast<int>(i) });
        lv_obj_t* card = GameCard::create(
            list,
            cardData,
            onCardClicked,
            &cardUserData.back());

        cards.push_back(card);
    }
}

void GameListView::updateCardSelectionState(size_t index)
{
    if (index >= cards.size() || index >= games.size())
        return;

    lv_obj_t* card = cards[index];
    const auto& game = games[index];

    GameCard::GameData cardData{
        game.title,
        game.description,
        game.difficulty,
        game.durationMin
    };

    if (static_cast<int>(index) == selectedIndex)
    {
        GameCard::setExpanded(card, cardData, onStartClicked, this);
    }
    else
    {
        GameCard::setCollapsed(card, cardData);
    }
}

void GameListView::update()
{
    if (!selectionChanged)
        return;

    for (size_t i = 0; i < cards.size(); ++i)
        updateCardSelectionState(i);

    selectionChanged = false;
}

void GameListView::onCardClicked(lv_event_t* e)
{
    auto* data = static_cast<CardUserData*>(lv_event_get_user_data(e));
    if (!data || !data->view)
        return;

    GameListView* view = data->view;
    int index = data->index;
    view->selectedIndex = (view->selectedIndex == index) ? -1 : index;
    view->selectionChanged = true;
}

void GameListView::onStartClicked(lv_event_t* e)
{
    auto* view = static_cast<GameListView*>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("start_game"));
}

void GameListView::onBackClicked(lv_event_t* e)
{
    auto* view = static_cast<GameListView*>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("back"));
}

void GameListView::destroy()
{
    // Clean up tracker (for any dynamic objects)
    getObjectTracker().cleanup();

    // Delete root (cascades to all children including cards)
    UICleanup::deleteRoot(screen);

    // Nullify remaining pointers
    header = nullptr;
    list = nullptr;
    btnBack = nullptr;

    // Clear dynamic object tracking
    cards.clear();
    cardUserData.clear();

    selectedIndex = -1;
    selectionChanged = false;
    pendingAction = GameListAction::None;
}
