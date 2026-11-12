#include "VolumeFeedback.h"
#include "systems/audio/AudioSystem.h"

void VolumeFeedback::bind(AudioSystem* system) {
    stop();
    audio = system;
    attempted = false;
}

void VolumeFeedback::request(AudioChannel target, AudioChannelPreferences value, uint32_t now) {
    if (!value.enabled || !value.volume) {
        stop();
        return;
    }
    channel = target;
    volume = value.volume;
    pending = !heard || channel != lastChannel || volume != lastVolume;
    update(now);
}

void VolumeFeedback::update(uint32_t now) {
    if (!pending || !audio || (attempted && uint32_t(now - attemptedAt) < INTERVAL_MS))
        return;
    if (source)
        audio->stopSource(source);
    // Generated PCM avoids SD reads and does not interrupt music or other FX.
    source = audio->beep(FREQUENCY_HZ, DURATION_MS, channel, SOURCE_VOLUME);
    attemptedAt = now;
    attempted = true;
    if (source) {
        pending = false;
        heard = true;
        lastChannel = channel;
        lastVolume = volume;
    }
    // Queue pressure is retried at the same bounded rate, without stealing voices.
}

void VolumeFeedback::stop() {
    if (audio && source)
        audio->stopSource(source);
    source = 0;
    pending = false;
    heard = false;
}
