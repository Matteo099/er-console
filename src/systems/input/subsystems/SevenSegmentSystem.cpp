#include "SevenSegmentSystem.h"

void SevenSegmentSystem::registerDriver(ISevenSegmentDriver *driver) { drivers.push_back(driver); }

bool SevenSegmentSystem::init()
{
    for (auto d : drivers)
        if (!d->init())
            return false;
    return true;
}

void SevenSegmentSystem::update() {}

void SevenSegmentSystem::shutdown()
{
    for (auto d : drivers)
    {
        d->shutdown();
    }
}

void SevenSegmentSystem::display(uint8_t index, const std::string &value)
{
    if (index >= drivers.size())
        return;
    auto *driver = drivers[index];
    driver->clear();
    uint8_t max = driver->digits();
    uint8_t pos = 0;
    for (char c : value)
    {
        if (pos >= max)
            break;
        bool dot = false;
        if (c == '.')
        {
            continue;
        }
        if (pos + 1 < value.size() && value[pos + 1] == '.')
        {
            dot = true;
        }
        driver->setDigit(pos, c, dot);
        pos++;
    }
}

void SevenSegmentSystem::clear(uint8_t index)
{
    if (index >= drivers.size())
        return;
    drivers[index]->clear();
}

void SevenSegmentSystem::brightness(uint8_t index, uint8_t value)
{
    if (index >= drivers.size())
        return;
    drivers[index]->setBrightness(value);
}