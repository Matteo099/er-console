#include "SettingsScene.h"
#include "log/Logger.h"

void SettingsScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering SettingsScene");
    view.setSoundEnabled(soundEnabled);
    view.create();
}

void SettingsScene::onUpdate()
{
    view.update();

    switch (view.consumeAction())
    {
    case SettingsAction::ToggleSound:
        soundEnabled = !soundEnabled;
        view.setSoundEnabled(soundEnabled);
        // TODO: persist setting here
        break;

    case SettingsAction::Back:
        request = SceneRequest(SceneChange::GoToMainMenu);
        break;

    default:
        break;
    }
}

void SettingsScene::onExit()
{
    view.destroy();
}

SceneRequest SettingsScene::getRequest() const
{
    return request;
}
