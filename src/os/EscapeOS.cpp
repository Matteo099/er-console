#include "EscapeOS.h"
#include "os/BootManager.h"
#include "scenes/boot/BootScene.h"
#include "scenes/boot/BootView.h"
#include "os/BootEvent.h"
#include "ui/lvgl/LVGLManager.h"
#include "devices/display/XPT2046Touch.h"

// Handle eventi globali
EventGroupHandle_t EscapeOS::systemEvents = nullptr;
QueueHandle_t EscapeOS::bootQueue = nullptr;

void EscapeOS::bootTask(void *param)
{

    BootManager boot;

    // Run full boot sequence
    boot.run();

    // Notify system that boot is completed
    xEventGroupSetBits(systemEvents, EVT_BOOT_DONE);

    // Boot task finished, delete itself
    vTaskDelete(nullptr);
}

void EscapeOS::uiTask(void* param) {

    LVGLManager::init();
    XPT2046Touch touch(TOUCH_CS_PIN);
    touch.init();
    lvgl_register_touch(&touch);

    BootView bootView;
    bootView.create();

    while (!(xEventGroupGetBits(systemEvents) & EVT_BOOT_DONE)) {

        BootEvent evt;
        while (xQueueReceive(bootQueue, &evt, 0) == pdTRUE) {
            bootView.update(evt.progress, evt.label);
        }

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(16));
    }

    vTaskDelete(nullptr);
}

void EscapeOS::inputTask(void *param)
{

    xEventGroupWaitBits(
        systemEvents,
        EVT_BOOT_DONE,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY);

    Serial.println("[INPUT] Ready");

    while (true)
    {
        // scan pulsanti
        // read touch raw
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void EscapeOS::audioTask(void *param)
{

    xEventGroupWaitBits(
        systemEvents,
        EVT_BOOT_DONE,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY);

    Serial.println("[AUDIO] Ready");

    while (true)
    {
        // wait for audio queue
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void EscapeOS::luaTask(void *param)
{

    xEventGroupWaitBits(
        systemEvents,
        EVT_BOOT_DONE,
        pdFALSE,
        pdTRUE,
        portMAX_DELAY);

    Serial.println("[LUA] Initializing VM");

    // init lua VM
    // load scripts from SD

    xEventGroupSetBits(systemEvents, EVT_LUA_READY);

    while (true)
    {
        // execute logic
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void EscapeOS::init()
{
    systemEvents = xEventGroupCreate();
    bootQueue = xQueueCreate(8, sizeof(BootEvent));
    createTasks();
}

void EscapeOS::createTasks()
{

    // Boot Task (core 0)
    xTaskCreatePinnedToCore(
        bootTask,
        "BootTask",
        4096,
        nullptr,
        configMAX_PRIORITIES - 1,
        nullptr,
        0);

    // UI Task (core 1)
    xTaskCreatePinnedToCore(
        uiTask,
        "UITask",
        8192,
        nullptr,
        3,
        nullptr,
        1);

    // Input Task
    xTaskCreatePinnedToCore(
        inputTask,
        "InputTask",
        4096,
        nullptr,
        3,
        nullptr,
        1);

    // Audio Task
    xTaskCreatePinnedToCore(
        audioTask,
        "AudioTask",
        4096,
        nullptr,
        2,
        nullptr,
        0);

    // Lua Task
    xTaskCreatePinnedToCore(
        luaTask,
        "LuaTask",
        8192,
        nullptr,
        1,
        nullptr,
        0);
}