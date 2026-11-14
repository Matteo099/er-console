#pragma once
class Scene;
void runHardwareDiagnostics();
Scene* createHardwareGameForDiagnostics(const char* mode);
void hardwareStatusForDiagnostics();
void roomIdForDiagnostics(unsigned pin);
