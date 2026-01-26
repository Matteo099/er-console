#pragma once

#include "../core/Bootable.h"

class InputManager : public Bootable {
public:
    void init() override;
    void update() override;
};
