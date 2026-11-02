#include "GameView.h"

#include "systems/ui/UIEvents.h"
#include "systems/ui/components/Screen.h"
#include "log/Logger.h"

namespace
{
    constexpr const char *TAG = "GameView";

    const char *stateToString(GameSubview state)
    {
        switch (state)
        {
        case GameSubview::Main:
            return "Main";
        case GameSubview::Loading:
            return "Loading";
        case GameSubview::Victory:
            return "Victory";
        case GameSubview::Fail:
            return "Fail";
        case GameSubview::GameObject:
            return "GameObject";
        default:
            return "Unknown";
        }
    }
}

void GameView::create()
{
    Log::info(TAG, "Creating GameView");

    screen = Screen::create();

    lv_obj_set_size(
        screen,
        LV_PCT(100),
        LV_PCT(100));

    mainSubview.create(screen);
    loadingSubview.create(screen);
    victorySubview.create(screen);
    failSubview.create(screen);
    gameObjectSubview.create(screen);

    auto callback =
        [this](const char *id)
    {
        Log::debug(TAG, "Button clicked: %s", id);
        emitEvent(ButtonClickedEvent(id));
    };

    mainSubview.setEventCallback(callback);
    victorySubview.setEventCallback(callback);
    failSubview.setEventCallback(callback);
    gameObjectSubview.setEventCallback(callback);

    showLoading();

    Log::info(TAG, "GameView created");
}

void GameView::destroy()
{
    Log::info(TAG, "Destroying GameView");

    mainSubview.destroy();
    loadingSubview.destroy();
    victorySubview.destroy();
    failSubview.destroy();
    gameObjectSubview.destroy();

    screen = nullptr;

    Log::info(TAG, "GameView destroyed");
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
    case GameSubview::GameObject:
        break;
    }
}

void GameView::setState(GameSubview state)
{
    if (currentState == state)
    {
        Log::debug(TAG, "State already %s", stateToString(state));
        return;
    }

    Log::info(
        TAG,
        "State change: %s -> %s",
        stateToString(currentState),
        stateToString(state));

    currentState = state;

    mainSubview.hide();
    loadingSubview.hide();
    victorySubview.hide();
    failSubview.hide();
    gameObjectSubview.hide();

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

    case GameSubview::GameObject:
        gameObjectSubview.show();
        break;
    }

    Log::info(
        "GameView",
        "screen hidden=%d",
        lv_obj_has_flag(screen, LV_OBJ_FLAG_HIDDEN));
}

void GameView::showMain()
{
    Log::debug(TAG, "showMain()");
    setState(GameSubview::Main);
}

void GameView::showLoading()
{
    Log::debug(TAG, "showLoading()");
    setState(GameSubview::Loading);
}

void GameView::showVictory()
{
    Log::debug(TAG, "showVictory()");
    setState(GameSubview::Victory);
}

void GameView::showFailure()
{
    Log::debug(TAG, "showFailure()");
    setState(GameSubview::Fail);
}

void GameView::showGameObject()
{
    Log::debug(TAG, "showGameObject()");
    setState(GameSubview::GameObject);
}

void GameView::setTime(uint32_t seconds)
{
    Log::debug(TAG, "Time updated: %lu", static_cast<unsigned long>(seconds));
    mainSubview.setTime(seconds);
}

void GameView::setRunning(bool running)
{
    Log::debug(TAG, "Running: %s", running ? "true" : "false");
    mainSubview.setRunning(running);
}

void GameView::setLoadingMessage(const std::string &message)
{
    Log::info(TAG, "Loading message: %s", message.c_str());
    loadingSubview.setMessage(message);
}

void GameView::setVictoryStats(const VictorySubview::GameStats &stats)
{
    Log::info(
        TAG,
        "Victory stats updated (elapsedSeconds=%d, pausedSeconds=%d, hintsUsed=%d, puzzlesSolved=%d)",
        stats.elapsedSeconds,
        stats.pausedSeconds,
        stats.hintsUsed,
        stats.puzzlesSolved);

    victorySubview.setStats(stats);
}

void GameView::setGameObjectData(const std::vector<GameObjectSubview::ObjectData> &data)
{
    Log::info(TAG, "Received %u game objects", static_cast<unsigned>(data.size()));
    gameObjectSubview.setObjects(data);
    mainSubview.setObjectsAvailable(!data.empty());
}
