#pragma once
#include "SdPack.h"
#include "config/Console.h"

namespace SdResources {
void mounted(bool available);
bool required();
bool current();
bool ready(const std::string& digest, const std::string& version);
std::string resolve(const char* logical, bool writing = false);
bool managed(const char* logical);
bool reserved(const char* path);
std::string root();
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
bool withGenerationForDiagnostics(const std::string& digest, const std::string& version,
                                  const std::function<void()>& inspect);
#endif
UpdateError install(UpdateInput& input, const UpdateRelease& release,
                    UpdateEngine::Progress progress, UpdateEngine::Cancel cancelled);
}  // namespace SdResources
