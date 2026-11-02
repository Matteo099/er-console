#pragma once
class UIFeedback
{
public:
    static bool soundEnabled() { return sound(); }
    static void setSoundEnabled(bool enabled) { sound() = enabled; }
private:
    static bool &sound() { static bool enabled = true; return enabled; }
};
