#include "SystemEventBus.h"

#include "os/BootEvent.h"
#include "systems/audio/AudioEvent.h"

// Handle eventi globali
EventGroupHandle_t SystemEventBus::systemEvents = nullptr;
QueueHandle_t SystemEventBus::bootQueue = nullptr;
QueueHandle_t SystemEventBus::audioQueue = nullptr;

void SystemEventBus::init()
{
    if (!systemEvents)
        systemEvents = xEventGroupCreate();

    if (!bootQueue)
        bootQueue = xQueueCreate(8, sizeof(BootEvent));

    if(!audioQueue)
        audioQueue = xQueueCreate(8, sizeof(AudioEvent));
}
