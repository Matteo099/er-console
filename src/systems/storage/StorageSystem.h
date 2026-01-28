#pragma once

#include "core/SystemService.h"
#include "drivers/storage/SDCardDriver.h"

class StorageSystem : public SystemService
{
public:
    explicit StorageSystem(SDCardDriver* driver);

    void init() override;
    bool isReady() const;
    void update() override {};

    const char* root() const { return "/sd"; }

private:
    SDCardDriver* driver;
    bool ready = false;
};
