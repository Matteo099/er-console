#pragma once

#include <string>
#include "core/SystemService.h"
#include "drivers/storage/SDCardDriver.h"
#include "drivers/storage/IFile.h"

class StorageSystem : public SystemService
{
public:
    StorageSystem();

    void init(void *params = nullptr) override;
    bool isReady() const;
    void update() override {};

    const char* root() const { return "/sd"; }
    virtual IFile* open(const char* path);
    bool readTextFile(const char* path, std::string& out);

private:
    SDCardDriver* driver;
    bool ready = false;
};
