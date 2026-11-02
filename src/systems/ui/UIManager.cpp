#include "UIManager.h"

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

void UIManager::init(void *params)
{
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

void UIManager::update()
{
    if (!preferencesLoaded && (xEventGroupGetBits(SystemEventBus::systemEvents) & EVT_STORAGE_READY))
        loadPreferences();
    // Advance LVGL timers & rendering
    lvgl.update();
    // Advance scene logic (non-blocking)
    scenes.update();
    UIDiagnostics::update(scenes);
}

void UIManager::loadPreferences()
{
    auto *storage = static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    DeviceConfigRepository repository;
    DeviceConfig config;
    repository.load(storage, config);
    UITheme::select(UITheme::fromName(config.ui.theme.c_str()));
    UIFeedback::setSoundEnabled(config.audio.enabled);
    AudioEvent event{};
    event.type = AudioEventType::SET_VOLUME;
    event.volume.volume = config.audio.enabled ? config.audio.volume : 0;
    xQueueSend(SystemEventBus::audioQueue, &event, 0);
    preferencesLoaded = (storage && storage->isReady()) ||
        (xEventGroupGetBits(SystemEventBus::systemEvents) & EVT_STORAGE_READY);
}
