#include "BootManager.h"

#include "os/BootEvent.h"
#include "core/SystemEventBus.h"
#include "log/Logger.h"

void BootManager::run()
{
    emitStep(BootStep::CORE_INIT, "Initializing core services");
    vTaskDelay(pdMS_TO_TICKS(100));

    // ---- RAINBOW LED ----
    emitStep(BootStep::RAINBOW_LED_INIT, "Initializing Rainbow LED");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_RAINBOWLED_INIT_REQ);
    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_RAINBOWLED_READY,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(100));

    // ---- DISPLAY / UI ----
    emitStep(BootStep::DISPLAY_INIT, "Initializing display");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_UI_INIT_REQ);
    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_UI_READY,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(100));

    // ---- STORAGE ----
    emitStep(BootStep::STORAGE_INIT, "Initializing storage");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_STORAGE_INIT_REQ);
    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_STORAGE_READY,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(100));

    // ---- AUDIO ----
    emitStep(BootStep::AUDIO_INIT, "Initializing audio");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_AUDIO_INIT_REQ);
    xEventGroupWaitBits(
        SystemEventBus::systemEvents,
        EVT_AUDIO_READY,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(100));

    // ---- INPUT ----
    emitStep(BootStep::INPUT_INIT, "Initializing input");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_INPUT_INIT_REQ);
    xEventGroupWaitBits(
        SystemEventBus::systemEvents,
        EVT_INPUT_READY,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(100));

    // ---- LUA ----
    emitStep(BootStep::LUA_INIT, "Initializing Lua");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_LUA_INIT_REQ);
    xEventGroupWaitBits(
        SystemEventBus::systemEvents,
        EVT_LUA_READY,
        pdTRUE,
        pdTRUE,
        portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(100));

    emitStep(BootStep::DONE, "Boot completed");
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_BOOT_DONE);
}

uint8_t BootManager::bootProgress(BootStep step)
{
    const uint8_t count = static_cast<uint8_t>(BootStep::COUNT);
    const uint8_t index = static_cast<uint8_t>(step);

    if (index >= count)
        return 100;

    return (index * 100) / count;
}

void BootManager::emitStep(BootStep step, const char *label)
{
    uint8_t progress = bootProgress(step);

    BootEvent evt;
    evt.progress = progress;
    evt.label = label;

    xQueueSend(SystemEventBus::bootQueue, &evt, portMAX_DELAY);

    Log::info(Log::BOOT_TAG, "%s (%d%%)", label, progress);
}