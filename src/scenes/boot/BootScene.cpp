#include "BootScene.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"

BootScene::BootScene() : imagesLoaded(false)
{
}

void BootScene::onEnter()
{
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
    }

    view.update();
}

void BootScene::onExit()
{
    // handled by SceneManager → view.destroy()
}
