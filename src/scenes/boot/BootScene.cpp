#include "BootScene.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"

void BootScene::onEnter()
{
    // View creation handled by SceneManager before animation
    // Nothing else needed here for now
}

void BootScene::onUpdate()
{
    BootEvent evt;
    while (xQueueReceive(SystemEventBus::bootQueue, &evt, 0) == pdTRUE)
    {
        view.setProgress(evt.progress, evt.label);
    }

    view.update();
}

void BootScene::onExit()
{
    // Cleanup is handled by SceneManager (view.destroy())
}
