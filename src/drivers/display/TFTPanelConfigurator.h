#ifndef TFT_PANEL_CONFIGURATOR_H
#define TFT_PANEL_CONFIGURATOR_H

#include <LovyanGFX.hpp>
#include "config/BoardConfig.h"

class TFTPanelConfigurator {
public:
  void configure(lgfx::Panel_ILI9341& panel_instance) {
    auto cfg = panel_instance.config();
    cfg.pin_cs = TFT_CS_PIN;   // TFT CS
    cfg.pin_rst = TFT_RST_PIN; // TFT RST
    cfg.pin_busy = -1; // Not used

    cfg.panel_width = 240;
    cfg.panel_height = 320;
    cfg.offset_x = 0;
    cfg.offset_y = 0;
    cfg.offset_rotation = 0;
    cfg.readable = true;
    cfg.invert = false;
    cfg.rgb_order = false;
    cfg.bus_shared = true; // Share SPI with touch

    panel_instance.config(cfg);
  }
};

#endif // TFT_PANEL_CONFIGURATOR_H
