#pragma once

#include "../core/Bootable.h"

class LuaManager : public Bootable {
public:
    void init() override;
    void update() override;
};
