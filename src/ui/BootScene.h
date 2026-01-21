#pragma once
#include "ui/Scene.h"
#include "devices/display/Display.h"

class BootScene : public Scene {
public:
    BootScene(Display* display);

    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

private:
    void render();

    Display* display;
    uint8_t progress = 0;
    const char* label = "Starting";
};
