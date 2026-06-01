#include "EscapeOS.h"

#include "core/SystemEventBus.h"
#include "log/Logger.h"

void xTaskPortal(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskPortal'");
    EscapeOS *os = static_cast<EscapeOS *>(params);

    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_STORAGE_READY | EVT_NETWORK_READY,
                        pdFALSE,
                        pdTRUE,
                        portMAX_DELAY);
    PortalSystem *portal = os->getPortalSystem();
    portal->init();

    while (os->isRunning())
    {
        portal->update();
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    vTaskDelete(nullptr);
}

void xTaskBoot(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskBoot'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    BootManager *boot = os->getBootManager();

    // Run full boot sequence
    boot->run();

    // Boot task finished, delete itself
    vTaskDelete(nullptr);
}

void xTaskRainbowLed(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskRainbowLed'");
    EscapeOS *os = static_cast<EscapeOS *>(params);

    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_RAINBOWLED_INIT_REQ,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);
    RainbowPixel *rainbow = os->getRainbowPixel();

    rainbow->init();
    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_RAINBOWLED_READY);

    const TickType_t frameDelay = pdMS_TO_TICKS(16); // ~60 FPS
    TickType_t lastWakeTime = xTaskGetTickCount();

    while (os->isRunning())
    {
        rainbow->update();
        vTaskDelayUntil(&lastWakeTime, frameDelay);
    }

    vTaskDelete(nullptr);
}

void xTaskUI(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskUI'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_UI_INIT_REQ,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);

    UIManager *ui = os->getUIManager();
    ui->init();

    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_UI_READY);

    while (os->isRunning())
    {
        ui->update();
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    vTaskDelete(nullptr);
}

void xTaskStorage(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskStorage'");
    EscapeOS *os = static_cast<EscapeOS *>(params);

    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_STORAGE_INIT_REQ,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);

    StorageSystem *storage = os->getStorageSystem();
    storage->init();

    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_STORAGE_READY);

    vTaskDelete(nullptr);
}

void xTaskNetwork(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskNetwork'");
    EscapeOS *os = static_cast<EscapeOS *>(params);

    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_NETWORK_INIT_REQ,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);

    NetworkSystem *network = os->getNetworkSystem();
    StorageSystem *storage = os->getStorageSystem();
    network->init(storage);

    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_NETWORK_READY);

    while (os->isRunning())
    {
        network->update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelete(nullptr);
}

void xTaskInput(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskInput'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_INPUT_INIT_REQ,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);

    InputSystem *input = os->getInputSystem();
    input->init();

    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_INPUT_READY);

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
    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_AUDIO_INIT_REQ,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);

    AudioSystem *audio = os->getAudioSystem();
    StorageSystem *storage = os->getStorageSystem();
    audio->init(storage);

    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_AUDIO_READY);

    while (os->isRunning())
    {
        audio->update();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void xTaskLua(void *params)
{
    Log::info(Log::OS_TAG, "Starting task 'xTaskLua'");
    EscapeOS *os = static_cast<EscapeOS *>(params);
    xEventGroupWaitBits(SystemEventBus::systemEvents,
                        EVT_LUA_INIT_REQ,
                        pdTRUE,
                        pdTRUE,
                        portMAX_DELAY);

    LuaSystem *lua = os->getLuaSystem();
    StorageSystem *storage = os->getStorageSystem();
    lua->init(storage);

    xEventGroupSetBits(SystemEventBus::systemEvents, EVT_LUA_READY);

    while (os->isRunning())
    {
        lua->update();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void EscapeOS::init()
{
    Log::init(Log::Level::DEBUG);
    Log::info(Log::OS_TAG, "EscapeOS initializing...");

    SystemEventBus::init();
    SystemEventBus::instance().registerService(SystemEventBus::SERVICE_STORAGE, &storageSystem);
    SystemEventBus::instance().registerService(SystemEventBus::SERVICE_INPUT, &inputSystem);
    SystemEventBus::instance().registerService(SystemEventBus::SERVICE_NETWORK, &networkSystem);
    SystemEventBus::instance().registerService(SystemEventBus::SERVICE_PORTAL, &portalSystem);
    running = true;

    createTasks();
}

void EscapeOS::createTasks()
{
    // RainbowLed Task (core 0)
    xTaskCreatePinnedToCore(
        xTaskRainbowLed,
        "RainbowLedTask",
        6144,
        this,
        configMAX_PRIORITIES - 2,
        nullptr,
        0);

    // Boot Task (core 0)
    xTaskCreatePinnedToCore(
        xTaskBoot,
        "BootTask",
        4096,
        this,
        configMAX_PRIORITIES - 1,
        nullptr,
        0);

    // UI Task (core 1)
    xTaskCreatePinnedToCore(
        xTaskUI,
        "UITask",
        8192,
        this,
        3,
        nullptr,
        1);

    // Storage Task
    xTaskCreatePinnedToCore(
        xTaskStorage,
        "StorageTask",
        8192,
        this,
        2,
        nullptr,
        1);

    // Network Task
    xTaskCreatePinnedToCore(
        xTaskNetwork,
        "NetworkTask",
        6144,
        this,
        2,
        nullptr,
        0);

    // Portal Task
    xTaskCreatePinnedToCore(
        xTaskPortal,
        "PortalTask",
        8192,
        this,
        2,
        nullptr,
        1);

    // Input Task
    xTaskCreatePinnedToCore(
        xTaskInput,
        "InputTask",
        4096,
        this,
        3,
        nullptr,
        1);

    // Audio Task
    xTaskCreatePinnedToCore(
        xTaskAudio,
        "AudioTask",
        4096,
        this,
        2,
        nullptr,
        0);

    // Lua Task
    xTaskCreatePinnedToCore(
        xTaskLua,
        "LuaTask",
        8192,
        this,
        1,
        nullptr,
        0);
}