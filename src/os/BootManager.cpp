#include "BootManager.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "log/Logger.h"

void BootManager::run()
{
    emitStep(BootStep::CORE_INIT, 5, "Initializing core services");
    vTaskDelay(pdMS_TO_TICKS(100));

    // ---- DISPLAY / UI ----
    emitStep(BootStep::DISPLAY_INIT, 20, "Initializing display");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_UI_INIT_REQ);
    xEventGroupWaitBits(
        SystemEventBus::systemEvents,
        EVT_UI_READY,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);

    // ---- AUDIO ----
    emitStep(BootStep::AUDIO_INIT, 40, "Initializing audio");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_AUDIO_INIT_REQ);
    xEventGroupWaitBits(
        SystemEventBus::systemEvents,
        EVT_AUDIO_READY,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);

    // ---- INPUT ----
    emitStep(BootStep::INPUT_INIT, 60, "Initializing input");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_INPUT_INIT_REQ);
    xEventGroupWaitBits(
        SystemEventBus::systemEvents,
        EVT_INPUT_READY,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);

    // ---- LUA ----
    emitStep(BootStep::LUA_INIT, 80, "Initializing Lua");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_LUA_INIT_REQ);
    xEventGroupWaitBits(
        SystemEventBus::systemEvents,
        EVT_LUA_READY,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);

    emitStep(BootStep::DONE, 100, "Boot completed");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_BOOT_DONE);
}

void BootManager::emitStep(BootStep step, uint8_t p, const char *label)
{
    BootEvent evt;
    evt.progress = p;
    evt.label = label;

    xQueueSend(SystemEventBus::bootQueue, &evt, portMAX_DELAY);

    Log::info(Log::BOOT_TAG, "%s (%d%%)", label, p);
}

uint8_t BootManager::getProgress() const
{
    return progress;
}

const char *BootManager::getCurrentStepName() const
{
    return stepLabel;
}