#include "TFTDisplay.h"
#include "log/Logger.h"
#include "config/BoardConfig.h"

TFTDisplay::TFTDisplay() : lgfx() {}

void TFTDisplay::init()
{
    Log::info(Log::DISPLAY_TAG, "Initializing TFTDisplay with LovyanGFX");

    // Initialize LovyanGFX
    lgfx.init();
    lgfx.initDMA();
    lgfx.setColorDepth(16);
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

    waitForFlush();
    // Match the panel's wire byte order once, then DMA the LVGL buffer directly.
    // rgb565_t would make LovyanGFX convert/copy it in small internal chunks.
    auto *pixels = reinterpret_cast<uint16_t *>(colorMap);
    for (int32_t i = 0; i < w * h; ++i) pixels[i] = __builtin_bswap16(pixels[i]);
    lgfx.startWrite();
    lgfx.setAddrWindow(x1, y1, w, h);
    lgfx.writePixelsDMA(reinterpret_cast<lgfx::swap565_t *>(colorMap), w * h);
    dmaPending = true;
}

void TFTDisplay::waitForFlush()
{
    if (!dmaPending) return;
    lgfx.waitDMA();
    lgfx.endWrite();
    dmaPending = false;
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
    waitForFlush(); // Touch shares SPI2 with the panel.
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
