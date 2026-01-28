#pragma once

#include "core/SystemService.h"

class InputSystem : public SystemService {
public:
    void init(void *params = nullptr) override;
    void update() override;
};
