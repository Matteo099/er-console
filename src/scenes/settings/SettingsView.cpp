#include "SettingsView.h"
#include "log/Logger.h"
#include "systems/ui/UIEvents.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"

void SettingsView::create()
{
    Log::debug(Log::UI_TAG, "Creating SettingsView");

    screen = Screen::create();
    auto layout = SceneLayout::create(screen, 70);
    brandImg = BrandHeader::create(layout.top);

    container = Container::create(layout.body, 220, 200, LV_ALIGN_TOP_MID, 0, 0);
    Container::setFlex(container);

    btnWifi = Button::create(container, "Wi-Fi", onWifiClicked, this, 200, 42, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_WIFI);
    lblWifi = Label::create(container, "Wi-Fi status: Unknown", &lv_font_montserrat_12, LV_TEXT_ALIGN_CENTER, 220);
    lv_obj_set_style_text_color(lblWifi, lv_color_hex(0x999999), 0);

    btnMonitor = Button::create(container, "Monitor", onMonitorClicked, this, 200, 42, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_SETTINGS);

    btnSound = Button::create(container, "Sound: OFF", onSoundClicked, this, 200, 42, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_MUTE);
    iconSound = lv_obj_get_child(btnSound, 0);
    lblSound = lv_obj_get_child(btnSound, 1);

    btnBack = BackButton::create(layout.footer, onBackClicked, this);
}

void SettingsView::update()
{
    // no animations yet
}

void SettingsView::destroy()
{
    // Clean up tracker (for any dynamic objects)
    getObjectTracker().cleanup();

    // Delete root (cascades to all children)
    UICleanup::deleteRoot(screen);

    // Nullify remaining pointers
    brandImg = nullptr;
    container = nullptr;
    btnWifi = nullptr;
    btnMonitor = nullptr;
    btnSound = nullptr;
    lblWifi = nullptr;
    lblSound = nullptr;
    btnBack = nullptr;

    pendingAction = SettingsAction::None;
}

void SettingsView::setSoundEnabled(bool enabled)
{
    if (lblSound)
    {
        lv_label_set_text(lblSound, enabled ? "Sound: ON" : "Sound: OFF");
    }
    if (iconSound)
    {
        lv_label_set_text(iconSound, enabled ? LV_SYMBOL_MUTE : LV_SYMBOL_VOLUME_MID);
    }
}

void SettingsView::setWiFiStatus(const std::string &status)
{
    if (!lblWifi)
        return;

    std::string text = std::string("Wi-Fi status: ") + status;
    lv_label_set_text(lblWifi, text.c_str());
}

void SettingsView::onWifiClicked(lv_event_t *e)
{
    auto *view = static_cast<SettingsView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("wifi_settings"));
}

void SettingsView::onSoundClicked(lv_event_t *e)
{
    auto *view = static_cast<SettingsView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("toggle_sound"));
}

void SettingsView::onMonitorClicked(lv_event_t *e)
{
    auto *view = static_cast<SettingsView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("monitor"));
}

void SettingsView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<SettingsView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("back"));
}
