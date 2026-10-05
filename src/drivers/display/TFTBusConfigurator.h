#ifndef TFT_BUS_CONFIGURATOR_H
#define TFT_BUS_CONFIGURATOR_H

#include <LovyanGFX.hpp>
#include "config/BoardConfig.h"

class TFTBusConfigurator {
public:
  void configure(lgfx::Bus_SPI& bus_instance, lgfx::Panel_ILI9341& panel_instance) {
    auto cfg = bus_instance.config();
    cfg.spi_host = SPI2_HOST; // ESP32-S3 VSPI/HSPI choice
    cfg.spi_mode = 0;
    cfg.freq_write = 20000000; // 20 MHz TFT SPI
    cfg.freq_read = 10000000;  // 10 MHz TFT read
    cfg.spi_3wire = false;     // 4-wire SPI
    cfg.use_lock = true;
    cfg.dma_channel = SPI_DMA_CH_AUTO;

    // TFT pins
    cfg.pin_sclk = SPI1_SCLK_PIN; // TFT SCLK (shared with touch)
    cfg.pin_mosi = SPI1_MOSI_PIN; // TFT MOSI (shared with touch)
    cfg.pin_miso = SPI1_MISO_PIN; // TFT MISO (shared with touch)
    cfg.pin_dc = TFT_DC_PIN;      // TFT D/C

    bus_instance.config(cfg);
    panel_instance.setBus(&bus_instance);
  }
};

#endif // TFT_BUS_CONFIGURATOR_H
