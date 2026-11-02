#include "SettingsScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "core/SystemEventBus.h"
#include "systems/network/NetworkSystem.h"
#include "systems/ui/UITheme.h"
#include "systems/ui/UIFeedback.h"
#include "systems/audio/AudioEvent.h"
#include "config/Console.h"
#include <Arduino.h>

void SettingsScene::onEnter()
{
    Log::info(Log::SCENE_TAG, "Entering SettingsScene");
    view.setEventHandler(this);
    storage = static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    repository.load(storage, config);
    soundEnabled = config.audio.enabled;
    UITheme::select(UITheme::fromName(config.ui.theme.c_str()));
    view.create();
    view.setSoundEnabled(soundEnabled);
    view.setTheme(UITheme::current());

    network = static_cast<NetworkSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    if (network)
    {
        const auto& status = network->getStatus();
        view.setWiFiStatus(status.message);
    }
}

void SettingsScene::onUpdate()
{
    if (dirty && millis() - changedAt >= 400) savePreferences();
    if (network)
    {
        view.setWiFiStatus(network->getStatus().message);
    }
    view.update();
}

void SettingsScene::onExit()
{
    if (dirty) savePreferences();
    view.destroy();
}

SceneRequest SettingsScene::getRequest() const
{
    return request;
}

bool SettingsScene::handleUIEvent(const UIEvent& event)
{
    if (event.getType() != ButtonClickedEvent::TYPE)
        return false;

    const auto& clicked = static_cast<const ButtonClickedEvent&>(event);
    if (clicked.buttonId == "theme_mint" || clicked.buttonId == "theme_sky" || clicked.buttonId == "theme_coral")
    {
        const auto id = clicked.buttonId == "theme_sky" ? UIThemeId::Sky :
                        clicked.buttonId == "theme_coral" ? UIThemeId::Coral : UIThemeId::Mint;
        UITheme::select(id);
        config.ui.theme = UITheme::name(id);
        view.setTheme(id);
        view.setPreferenceStatus("Tema applicato");
        dirty = true;
        changedAt = millis();
        return true;
    }
    if (clicked.buttonId == "toggle_sound")
    {
        soundEnabled = !soundEnabled;
        view.setSoundEnabled(soundEnabled);
        config.audio.enabled = soundEnabled;
        applyAudioPreference();
        dirty = true;
        changedAt = millis();
        return true;
    }

    if (clicked.buttonId == "wifi_settings")
    {
#if WIFI_ENABLED
        request = SceneRequest(SceneChange::GoToNetwork, SceneTransition::SlideLeft);
#endif
        return true;
    }

    if (clicked.buttonId == "monitor")
    {
        request = SceneRequest(SceneChange::GoToMonitor, SceneTransition::SlideLeft);
        return true;
    }

    if (clicked.buttonId == "back")
    {
        request = SceneRequest(SceneChange::GoToMainMenu, SceneTransition::SlideRight);
        return true;
    }

    return false;
}

void SettingsScene::applyAudioPreference()
{
    UIFeedback::setSoundEnabled(soundEnabled);
    AudioEvent event{};
    event.type = AudioEventType::SET_VOLUME;
    event.volume.volume = soundEnabled ? config.audio.volume : 0;
    xQueueSend(SystemEventBus::audioQueue, &event, 0);
}

void SettingsScene::savePreferences()
{
    config.ui.theme = UITheme::name(UITheme::current());
    config.audio.enabled = soundEnabled;
    const bool saved = repository.saveUiPreferences(config.ui.theme, soundEnabled);
    if (saved) Log::info(Log::UI_TAG, "UI preferences saved: theme=%s sound=%s",
                         config.ui.theme.c_str(), soundEnabled ? "on" : "off");
    view.setPreferenceStatus(saved ? "Preferenze salvate" : "Modifiche temporanee");
    if (!saved) Log::warn(Log::UI_TAG, "UI preferences applied in memory; NVS save unavailable");
    dirty = false;
}
