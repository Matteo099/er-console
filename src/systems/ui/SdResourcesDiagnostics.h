#pragma once
#include <functional>
void runSdResourcesDiagnostics(const std::function<void(bool, const char*)>& check);
