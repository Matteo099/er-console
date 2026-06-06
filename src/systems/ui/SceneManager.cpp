#include "systems/ui/SceneManager.h"
#include "log/Logger.h"

#include "scenes/menu/MainMenuScene.h"
#include "scenes/about/AboutScene.h"
#include "scenes/gamelist/GameListScene.h"
#include "scenes/settings/SettingsScene.h"
#include "scenes/monitor/MonitorScene.h"
#include "scenes/network/NetworkScene.h"
#include "scenes/game/GameScene.h"
#include "core/SystemEventBus.h"

static lv_scr_load_anim_t toLvgl(SceneTransition t)
{
    switch (t)
    {
    case SceneTransition::Fade:
        return LV_SCR_LOAD_ANIM_FADE_ON;

    case SceneTransition::SlideLeft:
        return LV_SCR_LOAD_ANIM_MOVE_LEFT;

    case SceneTransition::SlideRight:
        return LV_SCR_LOAD_ANIM_MOVE_RIGHT;

    case SceneTransition::None:
    default:
        return LV_SCR_LOAD_ANIM_NONE;
    }
}

void SceneManager::init(void *params)
{
    Log::info(Log::UI_TAG, "Initializing SceneManager");
    current = nullptr;
    pendingDelete = nullptr;
    pendingDeleteAtTick = 0;
}

void SceneManager::changeScene(Scene *next, SceneTransition transition)
{
    if (!next)
        return;

    if (pendingDelete)
    {
        pendingDelete->onExit();
        delete pendingDelete;
        pendingDelete = nullptr;
        pendingDeleteAtTick = 0;
    }

    Scene* previous = current;

    Log::info(Log::UI_TAG, "Changing scene to new scene.");
    current = next;

    current->onEnter();

    lv_scr_load_anim(
        current->getView().getScreen(),
        toLvgl(transition),
        300,
        0,
        false);

    pendingDelete = previous;
    pendingDeleteAtTick = previous ? (lv_tick_get() + 300) : 0;
}

void SceneManager::update()
{
    if (!current)
        return;

    if (pendingDelete && lv_tick_get() >= pendingDeleteAtTick)
    {
        pendingDelete->onExit();
        delete pendingDelete;
        pendingDelete = nullptr;
        pendingDeleteAtTick = 0;
    }

    current->onUpdate();

    SceneRequest req = current->getRequest();

    if (!req.isNone())
    {
        switch (req.change)
        {
        case SceneChange::GoToMainMenu:
            changeScene(new MainMenuScene(), req.transition);
            break;

        case SceneChange::GoToGameList:
            changeScene(new GameListScene(), req.transition);
            break;

        case SceneChange::GoToSettings:
            changeScene(new SettingsScene(), req.transition);
            break;

        case SceneChange::GoToMonitor:
            changeScene(new MonitorScene(), req.transition);
            break;

        case SceneChange::GoToAbout:
            changeScene(new AboutScene(), req.transition);
            break;

        case SceneChange::GoToNetwork:
            changeScene(new NetworkScene(), req.transition);
            break;

        case SceneChange::GoToGame:
        {
            GameScene* gameScene = new GameScene();
            // Extract game info from params
            if (req.params)
            {
                GameInfo* gameInfoPtr = static_cast<GameInfo*>(req.params);
                gameScene->setGameInfo(*gameInfoPtr);

                // Also set the GameView's LuaSystem reference
                LuaSystem* luaSystem = static_cast<LuaSystem*>(
                    SystemEventBus::instance().getService(SystemEventBus::SERVICE_LUA));
                if (luaSystem)
                {
                    luaSystem->setGameView(static_cast<GameView*>(&gameScene->getView()));
                }
            }
            changeScene(gameScene, req.transition);
            break;
        }

        default:
            break;
        }
    }
}
