#pragma once

#include "drivers/display/DisplayDriver.h"
#include "scenes/boot/BootView.h"
#include "os/BootEvent.h"
#include "os/EscapeOS.h"
#include "systems/ui/Scene.h"

class BootScene : public Scene
{
public:
    BootScene(DisplayDriver *display);

    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

private:
    DisplayDriver *display;
    BootView view; // interfaccia LVGL
    uint8_t progress = 0;
    const char *label = "Starting";
};
