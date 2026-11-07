#include "SettingsScene.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"
#include "core/SystemEventBus.h"
#include "systems/network/NetworkSystem.h"
#include "systems/audio/AudioSystem.h"
#include "systems/ui/UIFeedback.h"
#include "systems/ui/localization/I18n.h"
#include "config/Console.h"
#include <Arduino.h>

void SettingsScene::onEnter()
{
    view.setEventHandler(this);
    repository.loadCached(config);
    I18n::select(config.locale.language.c_str());
    UITheme::select(UITheme::fromName(config.ui.theme.c_str()));
    view.create(); view.selectTab(initialTab); view.setAudio(config.audio); view.setTheme(UITheme::current());
    network = static_cast<NetworkSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_NETWORK));
    audio = static_cast<AudioSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
    volumeFeedback.bind(audio);
}
void SettingsScene::onUpdate()
{
    if (view.currentTab() == SettingsView::Tab::Audio) volumeFeedback.update(millis());
    else volumeFeedback.stop();
    if (dirty && millis() - changedAt >= 400) savePreferences();
#if WIFI_ENABLED
    const auto state = network ? network->getStatus().state : NetworkSystem::State::Error;
    view.setWiFiStatus(tr(!network ? Text::NetworkUnavailable : state == NetworkSystem::State::Connected ? Text::Connected :
                         state == NetworkSystem::State::Connecting ? Text::Connecting : state == NetworkSystem::State::Error ? Text::ConnectionFailed : Text::Disconnected));
#else
    view.setWiFiStatus(tr(Text::WifiDisabled));
#endif
}
void SettingsScene::onExit() { volumeFeedback.stop(); if (dirty) savePreferences(); view.destroy(); }
void SettingsScene::onPreferencesChanged()
{
    if (dirty) return;
    volumeFeedback.stop();
    repository.loadCached(config);
    view.translate(); view.setAudio(config.audio); view.setTheme(UITheme::current());
}
SceneRequest SettingsScene::getRequest() const { return request; }
bool SettingsScene::handleUIEvent(const UIEvent &event)
{
    if (event.getType() == AudioPreferenceChangedEvent::TYPE)
    {
        const auto &changed = static_cast<const AudioPreferenceChangedEvent &>(event);
        const auto previous = changed.channel == AudioChannel::Music ? config.audio.music : config.audio.effects;
        (changed.channel == AudioChannel::Music ? config.audio.music : config.audio.effects) = changed.value;
        applyAudioPreference();
        if (changed.preview && previous.volume != changed.value.volume && view.currentTab() == SettingsView::Tab::Audio)
            volumeFeedback.request(changed.channel,changed.value,millis());
        else if (!changed.value.enabled || !changed.value.volume) volumeFeedback.stop();
    }
    else if (event.getType() == ButtonClickedEvent::TYPE)
    {
        const auto &id = static_cast<const ButtonClickedEvent &>(event).buttonId;
        if (id == "theme_mint" || id == "theme_sky" || id == "theme_coral")
        {
            const auto theme = id == "theme_sky" ? UIThemeId::Sky : id == "theme_coral" ? UIThemeId::Coral : UIThemeId::Mint;
            UITheme::select(theme); config.ui.theme = UITheme::name(theme); view.setTheme(theme);
        }
        else if (id == "language_it" || id == "language_en")
        {
            I18n::select(id == "language_en" ? I18n::Language::English : I18n::Language::Italian);
            config.locale.language = I18n::code(); view.translate();
        }
        else if (id == "wifi_settings") { request = SceneRequest(SceneChange::GoToNetwork, SceneTransition::SlideLeft); return true; }
        else if (id == "monitor") { request = SceneRequest(SceneChange::GoToMonitor, SceneTransition::SlideLeft); return true; }
        else if (id == "statistics") { request=SceneRequest(SceneChange::GoToStatistics,SceneTransition::SlideLeft); return true; }
        else if (id == "back") { request = SceneRequest(SceneChange::GoToMainMenu, SceneTransition::SlideRight); return true; }
        else return false;
    }
    else return false;
    dirty = true; changedAt = millis(); view.setPreferenceStatus(Text::Applied); return true;
}
void SettingsScene::applyAudioPreference()
{
    config.audio.enabled = config.audio.effects.enabled; config.audio.volume = config.audio.effects.volume;
    UIFeedback::setSoundEnabled(config.audio.effects.enabled && config.audio.effects.volume);
    if (audio) audio->configure(config.audio);
}
void SettingsScene::savePreferences()
{
    const bool saved = repository.savePreferences(config);
    view.setPreferenceStatus(saved ? Text::Saved : Text::Temporary);
    if (saved) Log::info(Log::UI_TAG, "UI preferences saved: theme=%s locale=%s music=%d/%u effects=%d/%u", config.ui.theme.c_str(),
                         config.locale.language.c_str(), config.audio.music.enabled, config.audio.music.volume,
                         config.audio.effects.enabled, config.audio.effects.volume);
    dirty = false;
}
