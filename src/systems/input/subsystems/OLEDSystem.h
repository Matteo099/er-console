#pragma once

#include <string>

#include "drivers/input/oled/IDisplay.h"
#include "ISubsystem.h"

#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

class OLEDSystem : public ISubsystem
{
public:
    explicit OLEDSystem(IDisplay& display);

    bool init() override;

    void update() override;

    bool isSupported() const;

    void clear();

    void showMessage(
        const std::string& text);

    std::string currentMessage() const;

private:
    IDisplay& display;

    mutable portMUX_TYPE lock =
        portMUX_INITIALIZER_UNLOCKED;

    bool supported = false;

    std::string message;

    bool dirty = false;
};