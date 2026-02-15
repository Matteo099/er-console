#pragma once

#include "config/BoardConfig.h"
#include "TouchController.h"
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

class XPT2046Touch : public TouchController {
public:
    explicit XPT2046Touch();

    void init() override;
    TouchPoint read() override;

private:
    XPT2046_Touchscreen touch;
    SPIClass touchSPI;
};
