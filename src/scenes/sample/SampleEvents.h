#pragma once

#include "systems/ui/UIEventSystem.h"

/**
 * @brief Event emitted when the play sound button is clicked
 */
class PlaySoundEvent : public UIEventType<PlaySoundEvent>
{
public:
    static constexpr const char* TYPE = "play_sound";

    PlaySoundEvent() = default;
    PlaySoundEvent(const std::string& soundPath) : soundPath(soundPath) {}

    std::string soundPath = "/fireball.wav";  // Default sound
};

/**
 * @brief Event emitted when a generic button is clicked
 */
class ButtonClickedEvent : public UIEventType<ButtonClickedEvent>
{
public:
    static constexpr const char* TYPE = "button_clicked";

    ButtonClickedEvent() = default;
    ButtonClickedEvent(const std::string& buttonId) : buttonId(buttonId) {}

    std::string buttonId;
};
