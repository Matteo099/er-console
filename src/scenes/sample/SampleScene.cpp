#include "SampleScene.h"
#include "log/Logger.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "scenes/menu/MainMenuScene.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"
#include <systems/audio/AudioEvent.h>


void SampleScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering SampleScene");
    view.create();
}

void SampleScene::onUpdate()
{
    if (view.consumeAction())
    {
        Log::info(Log::SCENE_TAG, "Play sound!!!!!");
        AudioEvent evt;
        evt.type = AudioEventType::PLAY_WAV;
        evt.wav.path = "/fireball.wav";
        xQueueSend(SystemEventBus::audioQueue, &evt, portMAX_DELAY);
    }
    view.update();
}

void SampleScene::onExit()
{
    view.destroy();
}

SceneRequest SampleScene::getRequest() const
{
    return SceneRequest();
}
