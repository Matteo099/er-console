#pragma once
#include "UpdateTypes.h"

namespace UpdateMetadata {
bool versionValid(const std::string& version);
// Return -1/0/1, ignoring build metadata. Invalid inputs are rejected separately.
int compareVersions(const std::string& left, const std::string& right);
bool trustedDownloadUrl(const std::string& url);
std::string readable(const char* text, size_t limit, bool& truncated);
UpdateError parseRelease(char* json, size_t length, UpdateRelease& release);
UpdateError parseManifest(char* json, size_t length, UpdateRelease& release,
                          const std::string& hardware, const std::string& layout, size_t slotSize);
}  // namespace UpdateMetadata
