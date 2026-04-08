#include "TFTDisplay.h"
#include "log/Logger.h"
#include "config/BoardConfig.h"

// Custom LGFX class with ILI9341 + XPT2046 SPI touch
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Touch_XPT2046 _touch_instance;

public:
    LGFX(void)
    {
        // SPI Bus configuration for TFT
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST; // HSPI
            cfg.spi_mode = 0;
            cfg.freq_write = 20000000; // 20 MHz TFT SPI
            cfg.freq_read = 10000000;  // 10 MHz TFT read
            cfg.spi_3wire = false;     // 4-wire SPI
            cfg.use_lock = true;
            cfg.dma_channel = SPI_DMA_CH_AUTO;

            // TFT pins
            cfg.pin_sclk = SPI1_SCLK_PIN;
            cfg.pin_mosi = SPI1_MOSI_PIN;
            cfg.pin_miso = SPI1_MISO_PIN;
            cfg.pin_dc = TFT_DC_PIN;

            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // TFT Panel configuration
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = TFT_CS_PIN;
            cfg.pin_rst = TFT_RST_PIN;
            cfg.pin_busy = -1;

            cfg.panel_width = 240;
            cfg.panel_height = 320;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.readable = true;
            cfg.invert = false;
            cfg.rgb_order = false;
            cfg.bus_shared = true; // Share SPI with touch

            _panel_instance.config(cfg);
        }

        // SPI Touch configuration
        {
            auto cfg = _touch_instance.config();

            cfg.x_min = 200;
            cfg.x_max = 3800;
            cfg.y_min = 3800;
            cfg.y_max = 200;

            cfg.spi_host = SPI2_HOST;
            cfg.freq = 1000000;           // 1 MHz safe for touch
            cfg.pin_sclk = SPI1_SCLK_PIN; // T_CLK
            cfg.pin_mosi = SPI1_MOSI_PIN; // T_DIN
            cfg.pin_miso = SPI1_MISO_PIN; // T_DO
            cfg.pin_cs = TOUCH_CS_PIN;    // T_CS
            cfg.pin_int = TOUCH_IRQ_PIN;  // T_IRQ

            cfg.bus_shared = true; // Shared with TFT
            cfg.offset_rotation = 0;

            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

TFTDisplay::TFTDisplay() : lgfx() {}

void TFTDisplay::init()
{
    Log::info(Log::DISPLAY_TAG, "Initializing TFTDisplay with LovyanGFX");

    // Initialize LovyanGFX
    lgfx.init();
    lgfx.setRotation(1);

    Log::info(Log::DISPLAY_TAG, "LovyanGFX initialized OK");

    // --- Display statistics ---
    int w = lgfx.width();
    int h = lgfx.height();
    int rotation = lgfx.getRotation();
    Log::debug(Log::DISPLAY_TAG, "Resolution: %dx%d", w, h);
    Log::debug(Log::DISPLAY_TAG, "Rotation: %d", rotation);

    // Access panel configuration
    auto *panel = lgfx.getPanel();
    if (panel)
    {
        auto cfg = panel->config(); // Panel_Config struct

        Log::debug(Log::DISPLAY_TAG, "Panel CS pin: %d", cfg.pin_cs);
        Log::debug(Log::DISPLAY_TAG, "Panel RST pin: %d", cfg.pin_rst);
        Log::debug(Log::DISPLAY_TAG, "Panel width: %d", cfg.panel_width);
        Log::debug(Log::DISPLAY_TAG, "Panel height: %d", cfg.panel_height);
        Log::debug(Log::DISPLAY_TAG, "Offset X/Y: %d/%d", cfg.offset_x, cfg.offset_y);
        Log::debug(Log::DISPLAY_TAG, "Offset rotation: %d", cfg.offset_rotation);
        Log::debug(Log::DISPLAY_TAG, "Panel readable: %s", cfg.readable ? "yes" : "no");
        Log::debug(Log::DISPLAY_TAG, "Panel invert: %s", cfg.invert ? "yes" : "no");
        Log::debug(Log::DISPLAY_TAG, "Bus shared: %s", cfg.bus_shared ? "yes" : "no");
    }

    Log::info(Log::DISPLAY_TAG, "Display debug info printed successfully");
}

void TFTDisplay::flush(
    int32_t x1, int32_t y1,
    int32_t x2, int32_t y2,
    uint8_t *colorMap)
{
    int32_t w = x2 - x1 + 1;
    int32_t h = y2 - y1 + 1;

    lgfx.startWrite();
    lgfx.setAddrWindow(x1, y1, w, h);
    lgfx.writePixels((lgfx::rgb565_t *)colorMap, w * h);
    lgfx.endWrite();
}

int TFTDisplay::width()
{
    auto panel = lgfx.getPanel();
    if (panel)
        return panel->config().panel_width;
    else
        return 240;
}
int TFTDisplay::height()
{
    auto panel = lgfx.getPanel();
    if (panel)
        return panel->config().panel_height;
    else
        return 320;
}

TouchPoint TFTDisplay::read()
{
    TouchPoint p{0, 0, false};
    int x, y;
    if (lgfx.getTouch(&x, &y))
    {
        p.x = x;
        p.y = y;
        p.pressed = true;
    }
    return p;
}
