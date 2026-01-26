#pragma once

#include <Arduino.h>
#include <lvgl.h>
#include "core/SystemService.h"

class LVGLContext : public SystemService
{
public:
    void init() override;
    void update() override;
};