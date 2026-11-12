#pragma once
class SceneManager;
struct TouchPoint;

namespace UIDiagnostics {
void update(SceneManager& scenes);
bool touchSample(TouchPoint& sample);
}  // namespace UIDiagnostics
