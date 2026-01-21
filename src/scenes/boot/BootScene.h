#pragma once
#include "devices/display/Display.h"
#include "BootView.h"
#include "os/BootEvent.h"
#include "os/EscapeOS.h"
#include "ui/Scene.h"

class BootScene : public Scene {
public:
    BootScene(Display* display);

    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

private:
    Display* display;
    BootView view;       // interfaccia LVGL
    uint8_t progress = 0;
    const char* label = "Starting";
};
