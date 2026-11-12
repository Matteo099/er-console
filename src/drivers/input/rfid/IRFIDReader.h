#pragma once

#include "RFIDTag.h"

class IRFIDReader {
  public:
    virtual ~IRFIDReader() = default;

    virtual bool init() = 0;

    virtual void shutdown() = 0;

    virtual bool readTag(RFIDTag& tag) = 0;

    virtual bool isAvailable() const = 0;
};