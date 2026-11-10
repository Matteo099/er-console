#include "SystemEventBus.h"

#include "os/BootEvent.h"
#include "systems/audio/AudioEvent.h"

// Handle eventi globali
EventGroupHandle_t SystemEventBus::systemEvents = nullptr;
QueueHandle_t SystemEventBus::bootQueue = nullptr;
QueueHandle_t SystemEventBus::audioQueue = nullptr;

SystemEventBus& SystemEventBus::instance()
{
    static SystemEventBus busInstance;
    return busInstance;
}

void SystemEventBus::registerService(int serviceId, SystemService* service)
{
    if (service)
    {
        services[serviceId] = service;
    }
}

SystemService* SystemEventBus::getService(int serviceId)
{
    auto it = services.find(serviceId);
    if (it != services.end())
    {
        return it->second;
    }
    return nullptr;
}

void SystemEventBus::init()
{
    if (!systemEvents)
        systemEvents = xEventGroupCreate();

    if (!bootQueue)
        bootQueue = xQueueCreate(8, sizeof(BootEvent));

    if(!audioQueue)
        audioQueue = xQueueCreate(8, sizeof(AudioEvent));
}
