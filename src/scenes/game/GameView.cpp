#include "GameView.h"

#include "systems/ui/UIEvents.h"
#include "systems/ui/components/Screen.h"

void GameView::create()
{
    screen = Screen::create();

    lv_obj_set_size(
        screen,
        LV_PCT(100),
        LV_PCT(100));

    mainSubview.create(screen);
    loadingSubview.create(screen);
    victorySubview.create(screen);
    failSubview.create(screen);

    auto callback =
        [this](const char *id)
        {
            emitEvent(ButtonClickedEvent(id));
        };

    mainSubview.setEventCallback(callback);
    victorySubview.setEventCallback(callback);
    failSubview.setEventCallback(callback);

    setState(GameSubview::Loading);
}

void GameView::destroy()
{
    mainSubview.destroy();
    loadingSubview.destroy();
    victorySubview.destroy();
    failSubview.destroy();

    screen = nullptr;
}

void GameView::update()
{
    switch (currentState)
    {
    case GameSubview::Main:
        mainSubview.update();
        break;

    case GameSubview::Loading:
    case GameSubview::Victory:
    case GameSubview::Fail:
        break;
    }
}

void GameView::setState(GameSubview state)
{
    currentState = state;

    mainSubview.hide();
    loadingSubview.hide();
    victorySubview.hide();
    failSubview.hide();

    switch (state)
    {
    case GameSubview::Main:
        mainSubview.show();
        break;

    case GameSubview::Loading:
        loadingSubview.show();
        break;

    case GameSubview::Victory:
        victorySubview.show();
        break;

    case GameSubview::Fail:
        failSubview.show();
        break;
    }
}

void GameView::showMain()
{
    setState(GameSubview::Main);
}

void GameView::showLoading()
{
    setState(GameSubview::Loading);
}

void GameView::showVictory()
{
    setState(GameSubview::Victory);
}

void GameView::showFailure()
{
    setState(GameSubview::Fail);
}

void GameView::setTime(uint32_t seconds)
{
    mainSubview.setTime(seconds);
}

void GameView::setRunning(bool running)
{
    mainSubview.setRunning(running);
}

void GameView::setLoadingMessage(const std::string &message)
{
    loadingSubview.setMessage(message);
}

void GameView::setVictoryStats(const VictorySubview::GameStats &stats)
{
    victorySubview.setStats(stats);
}