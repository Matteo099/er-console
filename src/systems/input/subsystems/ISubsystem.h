#pragma once

class ISubsystem
{
public:
    virtual ~ISubsystem() = default;
    virtual bool init() = 0;
    virtual void update() = 0;
};