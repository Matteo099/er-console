#pragma once

#include "systems/ui/View.h"
#include "GameListAction.h"
#include "../game/GameInfo.h"

#include <vector>

class GameListView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };
    GameInfo* getCurrentGame();
    void setGames(const std::vector<GameInfo> &games);
    void setLoadError(const std::string &message);

private:
    struct CardUserData
    {
        GameListView *view;
        int index;
    };

private:
    static void onStartClicked(lv_event_t *e);
    static void onBackClicked(lv_event_t *e);
    static void onPrevClicked(lv_event_t* e);
    static void onNextClicked(lv_event_t* e);
    
    void rebuildCurrentCard();

private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *header = nullptr;
    lv_obj_t *btnBack = nullptr;

    std::vector<GameInfo> games;

    lv_obj_t *cardContainer = nullptr;
    lv_obj_t *pageLabel = nullptr;

    lv_obj_t *btnPrev = nullptr;
    lv_obj_t *btnNext = nullptr;
    lv_obj_t *btnStart = nullptr;

    size_t currentIndex = 0;
    std::string loadError;

    GameListAction pendingAction = GameListAction::None;
};
