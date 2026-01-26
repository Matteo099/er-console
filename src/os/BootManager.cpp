#include "BootManager.h"
#include "os/BootState.h"
#include "log/Logger.h"
#include "os/BootEvent.h"
#include "os/EscapeOS.h"

void BootManager::run()
{
    Serial.begin(115200);
    setStep(BootStep::CORE_INIT, 5, "Initializing core services");
    vTaskDelay(pdMS_TO_TICKS(150));

    setStep(BootStep::DISPLAY_INIT, 20, "Initializing display");
    // UIManager->init();
    xEventGroupSetBits(EscapeOS::systemEvents, EscapeOS::EVT_UI_READY);
    vTaskDelay(pdMS_TO_TICKS(300));

    setStep(BootStep::STORAGE_INIT, 40, "Mounting storage");
    // FileSystem.mount();
    vTaskDelay(pdMS_TO_TICKS(300));

    setStep(BootStep::AUDIO_INIT, 60, "Initializing audio");
    // AudioManager.init();
    vTaskDelay(pdMS_TO_TICKS(300));

    setStep(BootStep::INPUT_INIT, 75, "Initializing input devices");
    // InputManager.init();
    vTaskDelay(pdMS_TO_TICKS(200));

    setStep(BootStep::LUA_INIT, 90, "Preparing Lua runtime");
    // LuaVM.init();
    vTaskDelay(pdMS_TO_TICKS(300));

    setStep(BootStep::DONE, 100, "Boot completed");
    vTaskDelay(pdMS_TO_TICKS(200));
}

void BootManager::setStep(BootStep step, uint8_t p, const char *label)
{
    BootEvent evt;
    evt.progress = p;
    evt.label = label;

    xQueueSend(EscapeOS::bootQueue, &evt, portMAX_DELAY);

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