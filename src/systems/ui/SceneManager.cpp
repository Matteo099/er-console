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
#include "config/Console.h"
#include "UIFrameStats.h"
#include <Arduino.h>

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

void SceneManager::oldScreenUnloadCb(lv_event_t* e)
{
    Scene* scene =
        static_cast<Scene*>(lv_event_get_user_data(e));

    if (!scene)
        return;

    Log::info(Log::UI_TAG, "Destroying old scene after screen unload");

    scene->onExit();
    delete scene;
}

void SceneManager::init(void *params)
{
    Log::info(Log::UI_TAG, "Initializing SceneManager");
    current = nullptr;
}

void SceneManager::changeScene(Scene* next, SceneTransition transition)
{
    if (!next)
        return;

    Scene* previous = current;

    Log::info(Log::UI_TAG, "Changing scene");

    current = next;
    const uint32_t prepareStarted = micros();
    current->onEnter();
    indicators.attachTo(current->getView().getScreen());
    UIFrameStats::prepared(micros() - prepareStarted);

    lv_obj_t* oldScreen = nullptr;

    if (previous)
    {
        oldScreen = previous->getView().getScreen();

        if (oldScreen)
        {
            lv_obj_add_event_cb(
                oldScreen,
                oldScreenUnloadCb,
                LV_EVENT_SCREEN_UNLOADED,
                previous);
        }
    }

#if !UI_SCENE_ANIMATIONS_ENABLED
    transition = SceneTransition::None;
#endif
    lv_scr_load_anim(
        current->getView().getScreen(),
        toLvgl(transition),
        transition == SceneTransition::None ? 0 : 140,
        0,
        true);

    // Special case:
    // If there was no previous scene, nothing will trigger deletion.
    // That's fine because there is nothing to delete.
}

void SceneManager::update()
{
    if (!current)
        return;

    current->onUpdate();
    indicators.update();
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
        case SceneChange::GoToSettingsSystem:
            changeScene(new SettingsScene(SettingsView::Tab::System), req.transition);
            break;

        case SceneChange::GoToMonitor:
            changeScene(new MonitorScene(), req.transition);
            break;

        case SceneChange::GoToAbout:
            changeScene(new AboutScene(), req.transition);
            break;

        case SceneChange::GoToNetwork:
#if WIFI_ENABLED
            changeScene(new NetworkScene(), req.transition);
#endif
            break;

        case SceneChange::GoToGame:
        {
            GameScene* gameScene = new GameScene();
            // Extract game info from params
            if (req.params)
            {
                GameInfo* gameInfoPtr = static_cast<GameInfo*>(req.params);
                gameScene->setGameInfo(*gameInfoPtr);
            }
            changeScene(gameScene, req.transition);
            break;
        }

        default:
            break;
        }
    }
}

void SceneManager::previewIndicators(unsigned mode)
{
    indicators.preview(mode == 0 ? DeviceStatusBar::StorageState::Ready :
                       mode == 1 ? DeviceStatusBar::StorageState::Degraded :
                       mode == 2 ? DeviceStatusBar::StorageState::Error : DeviceStatusBar::StorageState::Unavailable,
                       mode == 0 ? DeviceStatusBar::WiFiState::Connected :
                       mode == 1 ? DeviceStatusBar::WiFiState::Connecting :
                       mode == 2 ? DeviceStatusBar::WiFiState::Error : DeviceStatusBar::WiFiState::Disconnected);
}
