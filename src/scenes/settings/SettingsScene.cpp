#include "SettingsScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

void SettingsScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering SettingsScene");
    view.setEventHandler(this);
    view.setSoundEnabled(soundEnabled);
    view.create();
}

void SettingsScene::onUpdate()
{
    view.update();
}

void SettingsScene::onExit()
{
    view.destroy();
}

SceneRequest SettingsScene::getRequest() const
{
    return request;
}

bool SettingsScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "toggle_sound")
    {
        soundEnabled = !soundEnabled;
        view.setSoundEnabled(soundEnabled);
        return true;
    }

    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToMainMenu);
        return true;
    }

    return false;
}
