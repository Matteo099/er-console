#pragma once

class Bootable {
public:
    virtual ~Bootable() = default;

    virtual void init() = 0;
    virtual void update() = 0;
};
