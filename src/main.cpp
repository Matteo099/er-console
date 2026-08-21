#include <Arduino.h>

#include "config/BoardConfig.h"
#include "log/Logger.h"

#include "drivers/display/TFTDisplay.h"
#include "systems/ui/lvgl/LVGLContext.h"

// ============================================================
// GLOBAL OBJECTS
// ============================================================

static TFTDisplay display;
static LVGLContext lvgl;

// ============================================================
// TFT / LVGL TEST UI
// ============================================================

static void createTestUI()
{
    Log::info(
        Log::DISPLAY_TAG,
        "Creating LVGL test UI");

    // --------------------------------------------------------
    // Active screen
    // --------------------------------------------------------

    lv_obj_t *screen = lv_screen_active();

    // Dark background
    lv_obj_set_style_bg_color(
        screen,
        lv_color_hex(0x101010),
        LV_PART_MAIN);

    // --------------------------------------------------------
    // Title
    // --------------------------------------------------------

    lv_obj_t *title =
        lv_label_create(screen);

    lv_label_set_text(
        title,
        "ESP32 TFT TEST");

    lv_obj_set_style_text_color(
        title,
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN);

    lv_obj_set_style_text_font(
        title,
        &lv_font_montserrat_24,
        LV_PART_MAIN);

    lv_obj_align(
        title,
        LV_ALIGN_TOP_MID,
        0,
        20);

    // --------------------------------------------------------
    // Information label
    // --------------------------------------------------------

    lv_obj_t *info =
        lv_label_create(screen);

    lv_label_set_text(
        info,
        "ILI9341 + LovyanGFX + LVGL");

    lv_obj_set_style_text_color(
        info,
        lv_color_hex(0xCCCCCC),
        LV_PART_MAIN);

    lv_obj_align(
        info,
        LV_ALIGN_TOP_MID,
        0,
        60);

    // --------------------------------------------------------
    // Button
    // --------------------------------------------------------

    lv_obj_t *button =
        lv_button_create(screen);

    lv_obj_set_size(
        button,
        180,
        60);

    lv_obj_align(
        button,
        LV_ALIGN_CENTER,
        0,
        20);

    // Button label
    lv_obj_t *buttonLabel =
        lv_label_create(button);

    lv_label_set_text(
        buttonLabel,
        "TOUCH TEST");

    lv_obj_center(buttonLabel);

    // --------------------------------------------------------
    // Button event
    // --------------------------------------------------------

    lv_obj_add_event_cb(
        button,
        [](lv_event_t *e)
        {
            LV_UNUSED(e);

            Log::info(
                Log::DISPLAY_TAG,
                "TFT TEST BUTTON CLICKED");

            Serial.println(
                "TFT TEST BUTTON CLICKED");
        },
        LV_EVENT_CLICKED,
        nullptr);

    // --------------------------------------------------------
    // Status label
    // --------------------------------------------------------

    lv_obj_t *status =
        lv_label_create(screen);

    lv_label_set_text(
        status,
        "Display initialized");

    lv_obj_set_style_text_color(
        status,
        lv_color_hex(0x00FF00),
        LV_PART_MAIN);

    lv_obj_align(
        status,
        LV_ALIGN_BOTTOM_MID,
        0,
        -20);

    Log::info(
        Log::DISPLAY_TAG,
        "LVGL test UI created");
}

// ============================================================
// LVGL TASK
// ============================================================

static void lvglTask(void *parameter)
{
    LV_UNUSED(parameter);

    Log::info(
        Log::DISPLAY_TAG,
        "========================================");

    Log::info(
        Log::DISPLAY_TAG,
        "LVGL TASK STARTED");

    Log::info(
        Log::DISPLAY_TAG,
        "========================================");

    while (true)
    {
        // Process LVGL timers, input and rendering
        lvgl.update();

        // Give the CPU to other FreeRTOS tasks
        vTaskDelay(
            pdMS_TO_TICKS(5));
    }
}

// ============================================================
// SETUP
// ============================================================

void setup()
{
    // --------------------------------------------------------
    // Serial / Logger
    // --------------------------------------------------------

    Log::init(
        Log::Level::DEBUG);

    Log::info(
        Log::DISPLAY_TAG,
        "========================================");

    Log::info(
        Log::DISPLAY_TAG,
        "ESP32 TFT + LVGL TEST");

    Log::info(
        Log::DISPLAY_TAG,
        "========================================");

    // --------------------------------------------------------
    // Deselect SPI devices
    //
    // TFT, SD and touch share the SPI bus.
    // Keep all chip-select lines inactive during startup.
    // --------------------------------------------------------

    pinMode(
        TFT_CS_PIN,
        OUTPUT);

    digitalWrite(
        TFT_CS_PIN,
        HIGH);

    pinMode(
        TOUCH_CS_PIN,
        OUTPUT);

    digitalWrite(
        TOUCH_CS_PIN,
        HIGH);

    pinMode(
        SD_CS_PIN,
        OUTPUT);

    digitalWrite(
        SD_CS_PIN,
        HIGH);

    Log::info(
        Log::DISPLAY_TAG,
        "SPI chip-select lines initialized");

    // --------------------------------------------------------
    // Display
    // --------------------------------------------------------

    Log::info(
        Log::DISPLAY_TAG,
        "Initializing TFT display...");

    display.init();

    Log::info(
        Log::DISPLAY_TAG,
        "TFT display initialized");

    Log::info(
        Log::DISPLAY_TAG,
        "TFT resolution: %dx%d",
        display.width(),
        display.height());

    // --------------------------------------------------------
    // LVGL
    // --------------------------------------------------------

    Log::info(
        Log::DISPLAY_TAG,
        "Initializing LVGL...");

    lvgl.init(
        &display,
        &display);

    Log::info(
        Log::DISPLAY_TAG,
        "LVGL initialized");

    // --------------------------------------------------------
    // Create simple UI
    //
    // No Scene
    // No View
    // No SceneManager
    // --------------------------------------------------------

    createTestUI();

    // --------------------------------------------------------
    // Start LVGL task
    // --------------------------------------------------------

    BaseType_t result =
        xTaskCreate(
            lvglTask,
            "LVGL Task",
            8192,
            nullptr,
            2,
            nullptr);

    if (result != pdPASS)
    {
        Log::error(
            Log::DISPLAY_TAG,
            "FAILED TO CREATE LVGL TASK");

        return;
    }

    Log::info(
        Log::DISPLAY_TAG,
        "LVGL task started");

    Log::info(
        Log::DISPLAY_TAG,
        "========================================");

    Log::info(
        Log::DISPLAY_TAG,
        "TFT TEST READY");

    Log::info(
        Log::DISPLAY_TAG,
        "========================================");
}

// ============================================================
// LOOP
// ============================================================

void loop()
{
    // Everything is handled by the LVGL FreeRTOS task.
    //
    // Arduino loop does not need to do anything.

    vTaskDelay(
        portMAX_DELAY);
}