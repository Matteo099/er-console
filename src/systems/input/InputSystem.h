#pragma once

#include "core/SystemService.h"

class InputSystem : public SystemService {
public:
    void init() override;
    void update() override;
};
