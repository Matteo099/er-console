#include "UpdateView.h"
#include "systems/ui/components/Screen.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/UIEvents.h"
#include "config/Environment.h"
#include "config/Updates.h"
#include <cstring>

namespace
{
    void text(lv_obj_t *label, const std::string &value)
    {
        if (std::strcmp(lv_label_get_text(label), value.c_str()))
            Label::setText(label, value.c_str());
    }
    Text errorText(UpdateError error)
    {
        switch (error)
        {
        case UpdateError::Disabled:
            return Text::OtaDisabled;
        case UpdateError::Network:
            return Text::OtaNeedsWifi;
        case UpdateError::Clock:
            return Text::OtaClockError;
        case UpdateError::NotFound:
            return Text::OtaNotFound;
        case UpdateError::RateLimit:
            return Text::OtaRateLimit;
        case UpdateError::Metadata:
            return Text::OtaMetadataError;
        case UpdateError::Incompatible:
            return Text::OtaIncompatible;
        case UpdateError::Size:
            return Text::OtaSizeError;
        case UpdateError::Integrity:
            return Text::OtaIntegrityError;
        case UpdateError::Image:
            return Text::OtaImageError;
        case UpdateError::Flash:
            return Text::OtaFlashError;
        case UpdateError::Memory:
            return Text::OtaMemoryError;
        case UpdateError::Timeout:
            return Text::OtaTimeout;
        case UpdateError::GameActive:
            return Text::OtaGameActive;
        case UpdateError::BootPending:
            return Text::OtaBootPending;
        case UpdateError::Persistence:
            return Text::OtaActivationError;
        case UpdateError::RollbackUnavailable:
            return Text::OtaRollbackUnavailable;
        case UpdateError::Interrupted:
            return Text::OtaInterrupted;
        case UpdateError::Busy:
            return Text::OtaBusy;
        default:
            return Text::OtaHttpError;
        }
    }
    Text stateText(const UpdateSnapshot &value)
    {
        if (value.cancelling)
            return Text::OtaCancelling;
        switch (value.state)
        {
        case UpdateState::Checking:
            return Text::OtaChecking;
        case UpdateState::UpToDate:
            return Text::OtaUpToDate;
        case UpdateState::Available:
            return Text::OtaAvailable;
        case UpdateState::Downloading:
            return Text::OtaDownloading;
        case UpdateState::Verifying:
            return Text::OtaVerifying;
        case UpdateState::Installing:
            return Text::OtaInstalling;
        case UpdateState::Success:
            return Text::OtaRebooting;
        case UpdateState::Error:
            return errorText(value.error);
        case UpdateState::Cancelled:
            return Text::OtaCancelled;
        default:
            return value.confirmedUpdate ? Text::OtaConfirmed : Text::OtaIdle;
        }
    }
}
void UpdateView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen, 44, 48, false);
    auto *header = BrandHeader::create(layout.top, 100, tr(Text::OtaUpdates));
    title = lv_obj_get_child(header, 1);
    Container::setFlex(layout.body);
    lv_obj_set_style_pad_row(layout.body, 4, 0);
    auto *status = Container::create(layout.body, LV_PCT(100), 64);
    UITheme::card(status);
    Container::setFlex(status);
    lv_obj_set_style_pad_all(status, 6, 0);
    lv_obj_set_style_pad_row(status, 3, 0);
    statusLabel = Label::create(status, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    progressBar = lv_bar_create(status);
    lv_obj_set_size(progressBar, LV_PCT(100), 8);
    lv_bar_set_range(progressBar, 0, 100);
    transferLabel = Label::create(status, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    auto *details = Container::create(layout.body, LV_PCT(100), 0);
    lv_obj_set_flex_grow(details, 1);
    Container::setFlex(details);
    Container::setScrollable(details);
    lv_obj_set_style_pad_row(details, 8, 0);
    installed = Label::create(details, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    available = Label::create(details, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    auto *repository = Label::create(details, OTA_GITHUB_OWNER "/" OTA_GITHUB_REPOSITORY, &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(repository);
    networkButton = Button::create(details, tr(Text::Network), onButton, this, LV_PCT(100), 44);
    UITheme::secondary(networkButton);
    releaseTitle = Label::create(details, "", UITheme::heading(18), LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    notes = Label::create(details, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    Container::setFlex(layout.footer, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER);
    backButton = Button::create(layout.footer, tr(Text::Back), onButton, this, 96, 44);
    UITheme::secondary(backButton);
    checkButton = Button::create(layout.footer, tr(Text::OtaCheck), onButton, this, 104, 44);
    installButton = Button::create(layout.footer, tr(Text::OtaInstall), onButton, this, 104, 44);
    confirmation.create(screen, "", [this](bool accepted)
                        {
        confirmation.hide(); if (accepted) emitEvent(ButtonClickedEvent("confirm_install")); }, Text::OtaConfirmTitle, Text::OtaInstall, Text::Cancel);
}
void UpdateView::render(const UpdateSnapshot &value)
{
    text(title, tr(Text::OtaUpdates));
    text(statusLabel, tr(stateText(value)));
    if (value.state == UpdateState::Error)
        UITheme::danger(statusLabel);
    else
        UITheme::accent(statusLabel);
    const bool transfer = value.state == UpdateState::Downloading || value.state == UpdateState::Verifying || value.state == UpdateState::Installing || value.state == UpdateState::Success;
    lv_obj_set_flag(progressBar, LV_OBJ_FLAG_HIDDEN, !transfer);
    lv_obj_set_flag(transferLabel, LV_OBJ_FLAG_HIDDEN, !transfer);
    const auto percentage = [&](size_t amount)
    { return value.total ? unsigned(uint64_t(amount) * 100 / value.total) : 0; };
    lv_bar_set_value(progressBar, percentage(value.written), LV_ANIM_OFF);
    char counts[96];
    lv_snprintf(counts, sizeof(counts), tr(Text::OtaTransfer), percentage(value.downloaded), percentage(value.written));
    text(transferLabel, counts);
    text(installed, std::string(tr(Text::OtaInstalled)) + ": " + FW_VERSION);
    text(available, value.release.version.empty() ? "" : std::string(tr(Text::OtaLatest)) + ": " + value.release.version);
    text(releaseTitle, value.release.title.empty() ? "" : value.release.title);
    text(notes, value.release.notes + (value.release.notesTruncated ? std::string("\n") + tr(Text::OtaNotesTruncated) : ""));
    const bool busy = value.busy();
    const bool cancellable = busy && value.state != UpdateState::Installing && value.state != UpdateState::Success && !value.cancelling;
    text(lv_obj_get_child(checkButton, 0), tr(busy ? Text::Cancel : Text::OtaCheck));
    lv_obj_set_state(checkButton, LV_STATE_DISABLED, busy ? !cancellable : !OTA_ENABLED || !WIFI_ENABLED);
    lv_obj_set_state(backButton, LV_STATE_DISABLED, busy);
    lv_obj_set_state(installButton, LV_STATE_DISABLED, busy || !value.release.firmwareId || value.state == UpdateState::UpToDate);
    lv_obj_set_state(networkButton, LV_STATE_DISABLED, busy);
}
void UpdateView::askInstall(const std::string &version)
{
    char message[256];
    lv_snprintf(message, sizeof(message), tr(Text::OtaConfirmQuestion), version.c_str());
    confirmation.setQuestion(message);
    confirmation.show();
}
void UpdateView::destroy()
{
    confirmation.destroy();
    screen = title = statusLabel = progressBar = transferLabel = installed = available = releaseTitle = notes = nullptr;
    checkButton = installButton = backButton = networkButton = nullptr;
}
void UpdateView::onButton(lv_event_t *event)
{
    auto *self = static_cast<UpdateView *>(lv_event_get_user_data(event));
    auto *button = lv_event_get_target(event);
    const char *id = button == self->backButton ? "back" : button == self->checkButton ? "check"
                                                       : button == self->installButton ? "install"
                                                                                       : "wifi_settings";
    self->emitEvent(ButtonClickedEvent(id));
}
