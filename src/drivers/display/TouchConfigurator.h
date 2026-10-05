#ifndef TOUCH_CONFIGURATOR_H
#define TOUCH_CONFIGURATOR_H

#include <LovyanGFX.hpp>
#include "config/BoardConfig.h"

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
    cfg.pin_sclk = SPI1_SCLK_PIN; // T_CLK
    cfg.pin_mosi = SPI1_MOSI_PIN; // T_DIN
    cfg.pin_miso = SPI1_MISO_PIN; // T_DO
    cfg.pin_cs = TOUCH_CS_PIN;    // T_CS
    cfg.pin_int = -1;              // T_IRQ not connected in the tested setup

    cfg.bus_shared = true; // Shared with TFT
    cfg.offset_rotation = 0;

    touch_instance.config(cfg);
    panel_instance.setTouch(&touch_instance);
  }
};

#endif // TOUCH_CONFIGURATOR_H
