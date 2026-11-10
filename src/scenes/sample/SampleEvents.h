#pragma once

#include "systems/ui/UIEventSystem.h"
#include <string>

/**
 * @brief Sample-scene-specific event: Play sound on button press
 * 
 * This demonstrates how to create domain-specific events by inheriting
 * from UIEventType<T>. Note: Generic ButtonClickedEvent is in UIEvents.h
 */
class PlaySoundEvent : public UIEventType<PlaySoundEvent>
{
public:
    static constexpr const char* TYPE = "play_sound";

    PlaySoundEvent() = default;
    PlaySoundEvent(const std::string& soundPath) : soundPath(soundPath) {}

    std::string soundPath = "/fireball.wav";  // Default sound
};
