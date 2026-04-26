#pragma once

namespace SDLock
{
    void init();
    void lock();
    void unlock();

    class Guard
    {
    public:
        Guard();
        ~Guard();

        Guard(const Guard&) = delete;
        Guard& operator=(const Guard&) = delete;
    };
}
