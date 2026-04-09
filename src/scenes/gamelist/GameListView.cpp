#include "GameListView.h"
#include "log/Logger.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/Screen.h"
#include "systems/ui/components/ScrollList.h"

void GameListView::create()
{
    Log::debug(Log::UI_TAG, "Creating GameListView");
    screen = Screen::create();
    header = BrandHeader::create(screen);
    list = ScrollList::create(screen, 220, 180, LV_ALIGN_CENTER, 0, 10);
    btnBack = BackButton::create(screen, onBackClicked, this);
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

        lv_obj_t* card = lv_btn_create(list);
        lv_obj_set_width(card, 200);
        lv_obj_set_style_radius(card, 8, 0);
        lv_obj_set_style_pad_all(card, 8, 0);

        cardUserData.push_back({ this, static_cast<int>(i) });
        lv_obj_add_event_cb(card, onCardClicked, LV_EVENT_CLICKED, &cardUserData.back());

        lv_obj_t* title = Label::create(card, game.title.c_str(), &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT, 200);
        Align::setPosition(title, LV_ALIGN_TOP_LEFT, 0, 0);

        cards.push_back(card);
    }
}

void GameListView::rebuildCardContent(size_t index)
{
    lv_obj_t* card = cards[index];
    const auto& game = games[index];

    lv_obj_clean(card);

    lv_obj_t* title = Label::create(card, game.title.c_str(), &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT, 200);
    Align::setPosition(title, LV_ALIGN_TOP_LEFT, 0, 0);

    if (static_cast<int>(index) == selectedIndex)
    {
        lv_obj_t* desc = Label::create(card, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, 184);
        lv_label_set_text_fmt(
            desc,
            "%s\n\nDifficulty: %d/5\nDuration: %d min",
            game.description.c_str(),
            game.difficulty,
            game.durationMin);
        Align::setPosition(desc, LV_ALIGN_TOP_LEFT, 0, 28);

        lv_obj_t* btnStart = Button::create(card, "Start", onStartClicked, this, 80, 32, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        (void)btnStart;
    }
}

void GameListView::update()
{
    if (!selectionChanged)
        return;

    for (size_t i = 0; i < cards.size(); ++i)
        rebuildCardContent(i);

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

    view->pendingAction = GameListAction::StartGame;
}

void GameListView::onBackClicked(lv_event_t* e)
{
    auto* view = static_cast<GameListView*>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->pendingAction = GameListAction::Back;
}

GameListAction GameListView::consumeAction()
{
    GameListAction a = pendingAction;
    pendingAction = GameListAction::None;
    return a;
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
