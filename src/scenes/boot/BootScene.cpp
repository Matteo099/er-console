#include "BootScene.h"
#include "log/Logger.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "scenes/menu/MainMenuScene.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"
#include "systems/storage/StorageSystem.h"

void BootScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering BootScene");
    imagesLoaded = false;
    view.createTextOnly(); // SD not ready yet
}

void BootScene::onUpdate()
{
    // ---- Late-load images when storage is ready ----
    const bool storageInitFinished =
        xEventGroupGetBits(SystemEventBus::systemEvents) & EVT_STORAGE_READY;
    auto *storage = static_cast<StorageSystem *>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    if (!imagesLoaded && storageInitFinished && storage && storage->isReady())
    {
        Log::debug(Log::SCENE_TAG, "images can now be loaded - initializing LVGL image provider and loading boot images");
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
