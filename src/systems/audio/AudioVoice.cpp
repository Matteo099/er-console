#include "AudioVoice.h"

#include "systems/audio/AudioCache.h"

void AudioVoice::reset()
{
    //
    // Release cache ownership
    //
    if (cache && source)
    {
        cache->release(reinterpret_cast<MemoryAudioSource *>(source));
    }
    handle.reset();
    source = nullptr;
    cache = nullptr;
    state = State::Stopped;
    loop = false;
    priority = 0;
    volume = 1.0f;
    gain = 1.0f;
    targetGain = 1.0f;
    gainStep = 0.0f;
    interruptMode = InterruptMode::Stop;
    interruptedBy = 255;
    playId = 0;
}