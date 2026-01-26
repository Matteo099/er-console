#pragma once

#include "../../core/Bootable.h"

class AudioManager : public Bootable {
public:
    void init() override;
    void update() override;
};
