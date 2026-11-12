#pragma once

struct TouchPoint {
    int x;
    int y;
    bool pressed;
};

class TouchController {
  public:
    virtual ~TouchController() = default;
    virtual void init() = 0;
    virtual TouchPoint read() = 0;
};
