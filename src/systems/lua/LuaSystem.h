#pragma once

#include "core/SystemService.h"

class LuaSystem : public SystemService {
public:
    void init(void *params = nullptr) override;
    void update() override;
};
