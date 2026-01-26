#include "EscapeOS.h"
#include "BootEvent.h"
#include "../log/Logger.h"


// Handle eventi globali
EventGroupHandle_t EscapeOS::systemEvents = nullptr;
QueueHandle_t EscapeOS::bootQueue = nullptr;

void xTaskBoot(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskBoot'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    BootManager *boot = os->getBootManager();

    // Run full boot sequence
    boot->run();

    // Notify system that boot is completed
    xEventGroupSetBits(EscapeOS::systemEvents, EscapeOS::EVT_BOOT_DONE);

    // Boot task finished, delete itself
    vTaskDelete(nullptr);
}

void xTaskUI(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskUI'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    xEventGroupWaitBits(
        EscapeOS::systemEvents,
        EscapeOS::EVT_UI_READY,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY);

    UIManager *ui = os->getUIManager();
    ui->init();
    xEventGroupSetBits(EscapeOS::systemEvents, EscapeOS::EVT_UI_READY);

    while (os->isRunning())
    {
        ui->update();
        vTaskDelay(pdMS_TO_TICKS(16));
    }

    vTaskDelete(nullptr);
}

void xTaskInput(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskInput'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    xEventGroupWaitBits(
        EscapeOS::systemEvents,
        EscapeOS::EVT_UI_READY,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY);

    InputManager *input = os->getInputManager();
    input->init();

    while (os->isRunning())
    {
        input->update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void xTaskAudio(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskAudio'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    xEventGroupWaitBits(
        EscapeOS::systemEvents,
        EscapeOS::EVT_UI_READY,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY);

        
    AudioManager *audio = os->getAudioManager();
    audio->init();

    while (os->isRunning())
    {
        audio->update();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void xTaskLua(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskAudio'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    xEventGroupWaitBits(
        EscapeOS::systemEvents,
        EscapeOS::EVT_UI_READY,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY);

    Serial.println("[LUA] Initializing VM");

    LuaManager *lua = os->getLuaManager();
    lua->init();
    xEventGroupSetBits(EscapeOS::systemEvents, EscapeOS::EVT_LUA_READY);

    while (os->isRunning())
    {
        lua->update();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void EscapeOS::init()
{
    systemEvents = xEventGroupCreate();
    bootQueue = xQueueCreate(8, sizeof(BootEvent));
    running = true;

    createTasks();
}

void EscapeOS::createTasks()
{

    // Boot Task (core 0)
    xTaskCreatePinnedToCore(
        xTaskBoot,
        "BootTask",
        4096,
        nullptr,
        configMAX_PRIORITIES - 1,
        nullptr,
        0);

    // UI Task (core 1)
    xTaskCreatePinnedToCore(
        xTaskUI,
        "UITask",
        8192,
        nullptr,
        3,
        nullptr,
        1);

    // Input Task
    xTaskCreatePinnedToCore(
        xTaskInput,
        "InputTask",
        4096,
        nullptr,
        3,
        nullptr,
        1);

    // Audio Task
    xTaskCreatePinnedToCore(
        xTaskAudio,
        "AudioTask",
        4096,
        nullptr,
        2,
        nullptr,
        0);

    // Lua Task
    xTaskCreatePinnedToCore(
        xTaskLua,
        "LuaTask",
        8192,
        nullptr,
        1,
        nullptr,
        0);
}