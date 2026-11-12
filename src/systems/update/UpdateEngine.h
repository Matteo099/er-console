#pragma once
#include "UpdateTypes.h"
#include <functional>

class UpdateInput {
  public:
    virtual ~UpdateInput() = default;
    virtual int read(uint8_t* data, size_t capacity) = 0;
    virtual bool complete() const = 0;
    virtual int64_t contentLength() const = 0;
};

class UpdateFlash {
  public:
    virtual ~UpdateFlash() = default;
    virtual bool begin(size_t size) = 0;
    virtual bool write(const uint8_t* data, size_t size) = 0;
    virtual bool finish() = 0;
    virtual bool activate(const std::string& version) = 0;
    virtual void abort() = 0;
};

namespace UpdateEngine {
using Progress = std::function<void(UpdateState, size_t, size_t)>;
using Cancel = std::function<bool()>;
bool compatibleHeader(const uint8_t* data, size_t size);
UpdateError install(UpdateInput& input, UpdateFlash& flash, const UpdateRelease& release,
                    Progress progress, Cancel cancelled);
}  // namespace UpdateEngine
