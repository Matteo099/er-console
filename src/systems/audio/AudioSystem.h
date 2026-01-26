#pragma once

#include "core/SystemService.h"

class AudioSystem : public SystemService {
public:
    void init() override;
    void update() override;
};
