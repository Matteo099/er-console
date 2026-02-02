#include "GameScene.h"

void GameScene::onEnter()
{
    view.create();
    view.setTime(remainingSec);
    view.setRunning(false);
}

void GameScene::onUpdate()
{
    view.update();

    /* Handle buttons */
    if (view.startPressed)
    {
        view.startPressed = false;

        if (state == GameState::Ready || state == GameState::Paused)
        {
            state = GameState::Running;
            lastTickMs = lv_tick_get();
            view.setRunning(true);
        }
        else if (state == GameState::Running)
        {
            state = GameState::Paused;
            view.setRunning(false);
        }
    }

    if (view.cancelPressed)
    {
        request = SceneRequest(SceneChange::GoToMainMenu);
        return;
    }

    /* Timer */
    if (state == GameState::Running && remainingSec > 0)
    {
        uint32_t now = lv_tick_get();
        if (now - lastTickMs >= 1000)
        {
            lastTickMs += 1000;
            remainingSec--;
            view.setTime(remainingSec);

            if (remainingSec == 0)
                state = GameState::Finished;
        }
    }
}

void GameScene::onExit()
{
    view.destroy();
}

SceneRequest GameScene::getRequest() const
{
    return request;
}
