#pragma once

#include "core/SystemService.h"

class LuaSystem : public SystemService {
public:
    void init() override;
    void update() override;
};
