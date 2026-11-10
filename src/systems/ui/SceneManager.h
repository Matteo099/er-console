#pragma once

#include "core/SystemService.h"
#include "systems/ui/Scene.h"
#include "DeviceStatusBar.h"

class SceneManager : public SystemService
{
public:
    void init(void *params = nullptr) override;
    void update() override;

    void changeScene(Scene* next, SceneTransition transition);
    void previewIndicators(unsigned mode);
    void preferencesChanged() { if (current) current->onPreferencesChanged(); }

private:
    Scene* current = nullptr;
    DeviceStatusBar indicators;
    
    static void oldScreenUnloadCb(lv_event_t* e);
};
