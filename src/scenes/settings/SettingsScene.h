#pragma once

#include "systems/ui/Scene.h"
#include "scenes/settings/SettingsView.h"
#include "services/DeviceConfigRepository.h"

class SettingsScene : public Scene
{
public:    
    explicit SettingsScene(SettingsView::Tab tab = SettingsView::Tab::Appearance) : initialTab(tab) {}
    void onEnter() override;
    void onUpdate() override;
    void onExit() override;

    View& getView() override { return view; }
    SceneRequest getRequest() const override;

    bool handleUIEvent(const UIEvent& event) override;

private:
    SettingsView::Tab initialTab;
    SettingsView view;
    SceneRequest request = SceneRequest();

    // ---- Settings state (logic-owned) ----
    bool soundEnabled = true;
    class NetworkSystem* network = nullptr;
    StorageSystem *storage = nullptr;
    DeviceConfig config;
    DeviceConfigRepository repository;
    bool dirty = false;
    uint32_t changedAt = 0;
    void applyAudioPreference();
    void savePreferences();
};
