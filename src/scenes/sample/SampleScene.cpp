#include "SampleScene.h"
#include "log/Logger.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "scenes/menu/MainMenuScene.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"
#include <systems/audio/AudioEvent.h>
#include <cstdio>
#include "scenes/sample/SampleEvents.h"


void SampleScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering SampleScene");

    // Set up event handling - scene handles events from view
    view.setEventHandler(this);

    view.create();
}

void SampleScene::onUpdate()
{
    // No more polling! Events are handled immediately when emitted
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

// Event handling implementation
bool SampleScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() == PlaySoundEvent::TYPE)
    {
        const PlaySoundEvent& playEvent = static_cast<const PlaySoundEvent&>(event);

        Log::info(Log::SCENE_TAG, "Play sound!!!!!");
        AudioEvent evt{};
        evt.type = AudioEventType::PLAY_WAV;
        std::snprintf(evt.wav.path, sizeof(evt.wav.path), "%s", playEvent.soundPath.c_str());
        xQueueSend(SystemEventBus::audioQueue, &evt, portMAX_DELAY);

        return true;  // Event handled
    }

    return false;  // Event not handled
}
