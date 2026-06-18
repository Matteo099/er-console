#include "RFIDSystem.h"

#include <Arduino.h>

#include "log/Logger.h"

RFIDSystem::RFIDSystem(IRFIDReader &reader) : reader(reader) {}

bool RFIDSystem::init()
{
    supported = reader.init();
    if (!supported)
    {
        Log::warn(Log::INPUT_TAG, "RFID reader unavailable");
    }
    else
    {
        Log::info(Log::INPUT_TAG, "RFID reader initialized");
    }
    return supported;
}

void RFIDSystem::update() { poll(); }

bool RFIDSystem::isSupported() const { return supported; }

bool RFIDSystem::getLastTag(RFIDTag& tag) const
{
    portENTER_CRITICAL(&lock);

    if (!newTagAvailable)
    {
        portEXIT_CRITICAL(&lock);
        return false;
    }

    tag = lastTag;

    portEXIT_CRITICAL(&lock);

    return true;
}

bool RFIDSystem::consumeTag(RFIDTag &tag)
{
    portENTER_CRITICAL(&lock);
    if (!newTagAvailable)
    {
        portEXIT_CRITICAL(&lock);
        return false;
    }
    tag = lastTag;
    newTagAvailable = false;
    portEXIT_CRITICAL(&lock);
    return true;
}

/*
Future extensions:

using RFIDCallback =
    std::function<void(const RFIDTag&)>;

and add:

void setTagDetectedCallback(...);

so the application can react immediately to badge scans instead of polling consumeTag()
*/
void RFIDSystem::poll()
{
    if (!supported)
        return;
    const uint32_t now = millis();
    if (now - lastPollMs < POLL_INTERVAL_MS)
        return;
    lastPollMs = now;
    RFIDTag tag;
    if (!reader.readTag(tag))
        return;
    portENTER_CRITICAL(&lock);
    const bool duplicate = tag.uid == lastTag.uid && (now - lastTag.timestampMs) < DUPLICATE_TIMEOUT_MS;
    if (!duplicate)
    {
        lastTag = tag;
        newTagAvailable = true;
    }
    portEXIT_CRITICAL(&lock);
}