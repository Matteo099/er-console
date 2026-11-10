#include "BootScene.h"
#include "log/Logger.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"

void BootScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering BootScene");
    view.create();
}

void BootScene::onUpdate()
{
    // ---- Consume boot events ----
    BootEvent evt;
    while (xQueueReceive(SystemEventBus::bootQueue, &evt, 0) == pdTRUE)
    {
        view.setProgress(evt.progress, evt.label);

        if (evt.progress >= 100)
            bootCompleted = true;
    }

    if(bootCompleted && bootCompleteDelayTick > 0)
        bootCompleteDelayTick--; // assuming ~60 FPS

    view.update();
}

void BootScene::onExit()
{
    view.destroy();
}

SceneRequest BootScene::getRequest() const
{
    return bootCompleted && bootCompleteDelayTick <= 0
               ? SceneRequest(SceneChange::GoToMainMenu, SceneTransition::Fade)
               : SceneRequest();
}
