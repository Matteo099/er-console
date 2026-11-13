#pragma once
#include "UpdateEngine.h"

class SdPackSink {
  public:
    virtual ~SdPackSink() = default;
    virtual bool begin(size_t bytes, unsigned files) = 0;
    virtual bool open(const std::string& path, size_t size) = 0;
    virtual bool write(const uint8_t* data, size_t size) = 0;
    virtual bool finishFile(const uint8_t* sha256) = 0;
    virtual bool commit() = 0;
    virtual void abort() = 0;

    virtual UpdateError error() const {
        return UpdateError::Storage;
    }
};

namespace SdPack {
bool validPath(const std::string& path);
bool validDigest(const std::string& digest);
std::string sha256(const uint8_t* data, size_t size);
UpdateError install(UpdateInput& input, SdPackSink& sink, const UpdateRelease& release,
                    UpdateEngine::Progress progress, UpdateEngine::Cancel cancelled);
}  // namespace SdPack
