#pragma once

#include "systems/ui/UIEventSystem.h"
#include <string>

/**
 * @brief Event emitted when a button with a semantic ID is clicked.
 */
class ButtonClickedEvent : public UIEventType<ButtonClickedEvent>
{
public:
    static constexpr const char* TYPE = "button_clicked";

    ButtonClickedEvent() = default;
    ButtonClickedEvent(const std::string& buttonId) : buttonId(buttonId) {}

    std::string buttonId;
};
