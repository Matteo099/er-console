#include "UIManager.h"
#include "systems/update/UpdateSystem.h"

#include "log/Logger.h"
#include "scenes/boot/BootScene.h"
#include "scenes/sample/SampleScene.h"
#include "drivers/storage/ESP32SDCardDriver.h"
#include "systems/storage/StorageSystem.h"
#include "systems/ui/lvgl/LVGLImageProvider.h"
#include "config/BoardConfig.h"
#include "services/DeviceConfigRepository.h"
#include "systems/ui/UITheme.h"
#include "systems/ui/UIFeedback.h"
#include "systems/audio/AudioEvent.h"
#include "systems/ui/UIDiagnostics.h"
#include "core/SystemEventBus.h"
#include "localization/I18n.h"
#include "systems/audio/AudioSystem.h"
#include "localization/LocalizedText.h"

void UIManager::init(void* params) {
    Log::info(Log::UI_TAG, "Initializing UI Manager");

    // 1. Initialize display (LovyanGFX)
    display.init();

    // 2. Initialize LVGL core
    lvgl.init(&display, &display);
    loadPreferences();

    // Register LVGL filesystem access before any scene tries to load images
    LVGLImageProvider::init();

    // 3. Initialize scene manager
    scenes.init();

    // 4. Load initial scene (BOOT)
    // scenes.changeScene(new SampleScene(), SceneTransition::None);
    scenes.changeScene(new BootScene(), SceneTransition::Fade);

    Log::info(Log::UI_TAG, "UI Manager ready");
}

void UIManager::update() {
    if (!preferencesLoaded &&
        (xEventGroupGetBits(SystemEventBus::systemEvents) & EVT_STORAGE_READY))
        loadPreferences();
    const auto changedRevision = DeviceConfigRepository::revision();
    if (preferencesRevision != changedRevision) {
        DeviceConfig config;
        DeviceConfigRepository().loadCached(config);
        preferencesRevision = changedRevision;
        I18n::select(config.locale.language.c_str());
        UITheme::select(UITheme::fromName(config.ui.theme.c_str()));
        UIFeedback::setSoundEnabled(config.audio.effects.enabled && config.audio.effects.volume);
        auto* audio = static_cast<AudioSystem*>(
            SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
        if (audio)
            audio->configure(config.audio);
        scenes.preferencesChanged();
    }
    if (localeRevision != I18n::revision()) {
        localeRevision = I18n::revision();
        LocalizedText::refresh();
    }
    // Advance LVGL timers & rendering
    lvgl.update();
    // Advance scene logic (non-blocking)
    scenes.update();
    UIDiagnostics::update(scenes);
    UpdateSystem::instance().uiHeartbeat();
}

void UIManager::loadPreferences() {
    auto* storage = static_cast<StorageSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    DeviceConfigRepository repository;
    DeviceConfig config;
    repository.load(storage, config);
    I18n::select(config.locale.language.c_str());
    UITheme::select(UITheme::fromName(config.ui.theme.c_str()));
    UIFeedback::setSoundEnabled(config.audio.effects.enabled && config.audio.effects.volume);
    auto* audio = static_cast<AudioSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    if (audio)
        audio->configure(config.audio);
    preferencesLoaded = (storage && storage->isReady()) ||
                        (xEventGroupGetBits(SystemEventBus::systemEvents) & EVT_STORAGE_READY);
}
