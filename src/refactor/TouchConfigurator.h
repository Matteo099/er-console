#ifndef TOUCH_CONFIGURATOR_H
#define TOUCH_CONFIGURATOR_H

#include <LovyanGFX.hpp>

class TouchConfigurator {
public:
  void configure(lgfx::Touch_XPT2046& touch_instance, lgfx::Panel_ILI9341& panel_instance) {
    auto cfg = touch_instance.config();

    cfg.x_min = 200;
    cfg.x_max = 3800;
    cfg.y_min = 3800;
    cfg.y_max = 200;

    cfg.spi_host = SPI2_HOST;
    cfg.freq = 1000000; // 1 MHz safe for touch
    cfg.pin_sclk = 12;  // T_CLK
    cfg.pin_mosi = 11;  // T_DIN
    cfg.pin_miso = 13;  // T_DO
    cfg.pin_cs = 7;     // T_CS
    cfg.pin_int = 6;    // T_IRQ

    cfg.bus_shared = true; // Shared with TFT
    cfg.offset_rotation = 0;

    touch_instance.config(cfg);
    panel_instance.setTouch(&touch_instance);
  }
};

#endif // TOUCH_CONFIGURATOR_H