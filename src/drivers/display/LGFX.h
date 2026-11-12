#ifndef LGFX_H
#define LGFX_H

#include <LovyanGFX.hpp>
#include "TFTBusConfigurator.h"
#include "TFTPanelConfigurator.h"
#include "TouchConfigurator.h"

// ---------------------------
// Custom LGFX class with ILI9341 + FT5x06 SPI touch
// ---------------------------
class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Touch_XPT2046 _touch_instance;

    TFTBusConfigurator _bus_configurator;
    TFTPanelConfigurator _panel_configurator;
    TouchConfigurator _touch_configurator;

  public:
    LGFX(void) {
        // Configure bus
        _bus_configurator.configure(_bus_instance, _panel_instance);

        // Configure panel
        _panel_configurator.configure(_panel_instance);

        // Configure touch
        _touch_configurator.configure(_touch_instance, _panel_instance);

        setPanel(&_panel_instance);
    }
};

#endif  // LGFX_H