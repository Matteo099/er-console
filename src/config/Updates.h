#pragma once
#include <cstddef>
#include <cstdint>
#include "config/Console.h"
#ifndef OTA_GITHUB_OWNER
#define OTA_GITHUB_OWNER "Matteo099"
#endif
#ifndef OTA_GITHUB_REPOSITORY
#define OTA_GITHUB_REPOSITORY "er-console"
#endif
#ifndef OTA_HARDWARE_ID
#define OTA_HARDWARE_ID "esp32-s3-devkitc-1-n16r8v"
#endif
#ifndef OTA_PARTITION_TABLE_SHA256
#define OTA_PARTITION_TABLE_SHA256 ""
#endif

namespace UpdateConfig {
constexpr size_t JSON_LIMIT = 64 * 1024, MANIFEST_LIMIT = 8 * 1024;
constexpr uint32_t IO_TIMEOUT_MS = 15000, OPERATION_TIMEOUT_MS = 10 * 60 * 1000;
}  // namespace UpdateConfig
