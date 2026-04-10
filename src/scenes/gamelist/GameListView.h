#pragma once

#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"
#include "GameListAction.h"
#include "GameInfo.h"

#include <vector>

class GameListView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };

    void setGames(const std::vector<GameInfo> &games);

    bool hasSelectionChanged() const { return selectionChanged; }
    int getSelectedIndex() const { return selectedIndex; }

private:
    struct CardUserData
    {
        GameListView *view;
        int index;
    };

private:
    static void onCardClicked(lv_event_t *e);
    static void onStartClicked(lv_event_t *e);
    static void onBackClicked(lv_event_t *e);

    void rebuildCards();
    void updateCardSelectionState(size_t index);

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *header = nullptr;
    lv_obj_t *list = nullptr;
    lv_obj_t *btnBack = nullptr;

    std::vector<GameInfo> games;
    std::vector<lv_obj_t*> cards;
    std::vector<CardUserData> cardUserData;

    int selectedIndex = -1;
    bool selectionChanged = false;

    GameListAction pendingAction = GameListAction::None;
};
