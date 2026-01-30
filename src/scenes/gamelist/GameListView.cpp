#include "GameListView.h"

#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/BackButton.h"

void GameListView::create()
{
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    BrandHeader::create(screen);

    // ---- Scrollable list ----
    list = lv_obj_create(screen);
    lv_obj_set_size(list, 220, 180);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_flex_flow(list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_scroll_dir(list, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_AUTO);

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

        lv_obj_add_event_cb(
            card,
            onCardClicked,
            LV_EVENT_CLICKED,
            &cardUserData.back());

        // ---- Title ----
        lv_obj_t* title = lv_label_create(card);
        lv_label_set_text(title, game.title.c_str());
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

        cards.push_back(card);
    }
}

void GameListView::rebuildCardContent(size_t index)
{
    lv_obj_t* card = cards[index];
    const auto& game = games[index];

    lv_obj_clean(card);

    // ---- Title ----
    lv_obj_t* title = lv_label_create(card);
    lv_label_set_text(title, game.title.c_str());
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

    // ---- Expanded content ----
    if (static_cast<int>(index) == selectedIndex)
    {
        lv_obj_t* desc = lv_label_create(card);
        lv_label_set_text_fmt(
            desc,
            "%s\n\nDifficulty: %d/5\nDuration: %d min",
            game.description.c_str(),
            game.difficulty,
            game.durationMin);

        lv_obj_set_width(desc, 184);
        lv_obj_align(desc, LV_ALIGN_TOP_LEFT, 0, 28);

        // ---- Start button ----
        lv_obj_t* btnStart = lv_btn_create(card);
        lv_obj_set_size(btnStart, 80, 32);
        lv_obj_align(btnStart, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

        lv_obj_add_event_cb(
            btnStart,
            onStartClicked,
            LV_EVENT_CLICKED,
            this);

        lv_obj_t* lbl = lv_label_create(btnStart);
        lv_label_set_text(lbl, "Start");
        lv_obj_center(lbl);
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
    auto* data =
        static_cast<CardUserData*>(lv_event_get_user_data(e));

    if (!data || !data->view)
        return;

    GameListView* view = data->view;
    int index = data->index;

    view->selectedIndex =
        (view->selectedIndex == index) ? -1 : index;

    view->selectionChanged = true;
}

void GameListView::onStartClicked(lv_event_t* e)
{
    auto* view =
        static_cast<GameListView*>(lv_event_get_user_data(e));

    if (!view)
        return;

    view->pendingAction = GameListAction::StartGame;
}

void GameListView::onBackClicked(lv_event_t* e)
{
    auto* view =
        static_cast<GameListView*>(lv_event_get_user_data(e));

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
    if (screen)
        lv_obj_del(screen);

    screen = nullptr;
    list = nullptr;
    btnBack = nullptr;

    cards.clear();
    cardUserData.clear();

    selectedIndex = -1;
    selectionChanged = false;
    pendingAction = GameListAction::None;
}
