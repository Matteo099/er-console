#pragma once
#include <cstddef>
class Scene;
void runUpdateDiagnostics();
Scene* createUpdatePreview(unsigned mode);
void runNativeUpdateFlashDiagnostics(size_t size, const char* sha256);
