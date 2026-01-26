#pragma once
#include <stdint.h>

struct TouchPoint {
    int x;
    int y;
    bool pressed;
};

class TouchController {
public:
    virtual ~TouchController() {}
    virtual void init() = 0;
    virtual TouchPoint read() = 0;
};
