#pragma once

#include "core/SystemService.h"

class UIManager : public SystemService
{
public:
    void init() override;
    void update() override;
};