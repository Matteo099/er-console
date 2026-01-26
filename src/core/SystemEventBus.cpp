#include "SystemEventBus.h"

#include "os/BootEvent.h"

// Handle eventi globali
EventGroupHandle_t SystemEventBus::systemEvents = nullptr;
QueueHandle_t SystemEventBus::bootQueue = nullptr;

void SystemEventBus::init()
{
    if (!systemEvents)
        systemEvents = xEventGroupCreate();

    if (!bootQueue)
        bootQueue = xQueueCreate(8, sizeof(BootEvent));
}
