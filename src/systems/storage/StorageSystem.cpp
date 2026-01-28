#include "StorageSystem.h"

StorageSystem::StorageSystem(SDCardDriver* d)
    : driver(d) {}

void StorageSystem::init()
{
    ready = driver->init();
}

bool StorageSystem::isReady() const
{
    return ready;
}
