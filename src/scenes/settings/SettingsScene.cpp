#include "SettingsScene.h"

void SettingsScene::onEnter()
{
    view.create();
    view.setSoundEnabled(soundEnabled);
}

void SettingsScene::onUpdate()
{
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
