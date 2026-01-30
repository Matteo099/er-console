#include "BootScene.h"
#include "log/Logger.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "scenes/menu/MainMenuScene.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"

void BootScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering BootScene");
    imagesLoaded = false;
    view.createTextOnly(); // SD not ready yet
}

void BootScene::onUpdate()
{
    // ---- Late-load images when storage is ready ----
    if (!imagesLoaded &&
        (xEventGroupGetBits(SystemEventBus::systemEvents) & EVT_STORAGE_READY))
    {
        LVGLImageProvider::init(); // now SD is available
        view.loadImages();         // attach logo + brand
        imagesLoaded = true;
    }

    // ---- Consume boot events ----
    BootEvent evt;
    while (xQueueReceive(SystemEventBus::bootQueue, &evt, 0) == pdTRUE)
    {
        view.setProgress(evt.progress, evt.label);

        if (evt.progress >= 100)
            bootCompleted = true;
    }
}

void BootScene::onExit()
{
    view.destroy();
}

SceneRequest BootScene::getRequest() const
{
    return bootCompleted
               ? SceneRequest(SceneChange::GoToMainMenu)
               : SceneRequest();
}
