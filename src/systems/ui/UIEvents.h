#pragma once

#include "systems/ui/UIEventSystem.h"
#include <string>
#include "systems/audio/AudioPreferences.h"

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

class AudioPreferenceChangedEvent : public UIEventType<AudioPreferenceChangedEvent>
{
public:
    static constexpr const char *TYPE = "audio_preference_changed";
    AudioPreferenceChangedEvent(AudioChannel channel, AudioChannelPreferences value, bool preview = false) : channel(channel), value(value), preview(preview) {}
    AudioChannel channel;
    AudioChannelPreferences value;
    bool preview; // Slider interaction, rather than a toggle or configuration refresh.
};
