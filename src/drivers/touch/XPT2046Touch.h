#pragma once
#include "TouchController.h"
#include <XPT2046_Touchscreen.h>

class XPT2046Touch : public TouchController {
public:
    explicit XPT2046Touch(uint8_t csPin);

    void init() override;
    TouchPoint read() override;

private:
    XPT2046_Touchscreen ts;
};
