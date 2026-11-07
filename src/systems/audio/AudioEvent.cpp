#include "AudioEvent.h"
#include "AudioSystem.h"

AudioSourceId playAudioFile(const char *path, bool loop, AudioChannel channel, uint8_t volume)
{
    auto *audio = static_cast<AudioSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    return audio ? audio->play(path,loop,channel,volume) : 0;
}
bool stopAudioSource(AudioSourceId id)
{
    auto *audio = static_cast<AudioSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    return audio && audio->stopSource(id);
}
bool isAudioSourcePlaying(AudioSourceId id)
{
    auto *audio = static_cast<AudioSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    return audio && audio->isPlaying(id);
}

bool sendAudioEvent(const char *path, AudioEventType type, AudioChannel channel)
{
    auto *audio = static_cast<AudioSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    if (!audio) return false;
    if (type == AudioEventType::STOP) { audio->stop(channel); return true; }
    if (type == AudioEventType::PRELOAD) return audio->preload(path);
    if (type != AudioEventType::PLAY_WAV && type != AudioEventType::PLAY_LOOP_WAV) return false;
    return audio->play(path,type == AudioEventType::PLAY_LOOP_WAV,channel) != 0;
}
