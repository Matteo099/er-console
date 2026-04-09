#include "AboutScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

void AboutScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering AboutScene");
    view.setEventHandler(this);
    view.create();
}

void AboutScene::onUpdate()
{
    view.update();
}

void AboutScene::onExit()
{
    view.destroy();
}

SceneRequest AboutScene::getRequest() const
{
    return request;
}

bool AboutScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToMainMenu);
        return true;
    }
    return false;
}
