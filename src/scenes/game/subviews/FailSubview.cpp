#include "systems/ui/localization/I18n.h"
#include "FailSubview.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "log/Logger.h"
#include "systems/ui/components/SceneLayout.h"

namespace
{
    constexpr const char *TAG = "FailSubview";
}

void FailSubview::create(lv_obj_t *parent)
{
    Log::info(TAG, "Creating fail subview");

    root = Container::create(
        parent,
        LV_PCT(100),
        LV_PCT(100));

    auto layout = SceneLayout::create(root, 44, 48, false);
    titleLabel = Label::create(layout.top, tr(Text::TimeExpired), UITheme::heading(26),
                               LV_TEXT_ALIGN_CENTER, LV_PCT(100));
    UITheme::danger(titleLabel);
    lv_obj_center(titleLabel);
    messageLabel = Label::create(layout.body, "", &lv_font_montserrat_14,
                                 LV_TEXT_ALIGN_CENTER, LV_PCT(100));
    UITheme::muted(messageLabel);
    lv_obj_center(messageLabel);
    Container::setFlex(layout.footer, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(layout.footer, 8, 0);
    retryBtn = Button::create(layout.footer, tr(Text::Retry), onRetry, this, 132, 44,
                              LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_REFRESH);
    backBtn = Button::create(layout.footer, tr(Text::Back), onBack, this, 140, 44,
                             LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_LEFT);
    UITheme::secondary(backBtn);

    hide();

    Log::info(TAG, "Fail subview created");
}

void FailSubview::destroy()
{
    Log::info(TAG, "Destroying fail subview");

    root = nullptr;
    titleLabel = messageLabel = nullptr;
    retryBtn = nullptr;
    backBtn = nullptr;

    Log::info(TAG, "Fail subview destroyed");
}

void FailSubview::show()
{
    Log::debug(TAG, "Showing fail subview");
    if (!root || !lv_obj_has_flag(root,LV_OBJ_FLAG_HIDDEN)) return;
    lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
    playAudioFile(AUDIO_GAME_OVER_FILE, false, AudioChannel::Effects);
}

void FailSubview::hide()
{
    Log::debug(TAG, "Hiding fail subview");

    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void FailSubview::setEventCallback(EventCallback callback)
{
    Log::debug(TAG, "Event callback registered");
    eventCallback = std::move(callback);
}

void FailSubview::onRetry(lv_event_t *e)
{
    auto *view =
        static_cast<FailSubview *>(
            lv_event_get_user_data(e));

    if (!view)
    {
        Log::warn(TAG, "Retry callback received null view");
        return;
    }

    Log::debug(TAG, "Retry button clicked");

    if (view->eventCallback)
        view->eventCallback("retry");
}

void FailSubview::onBack(lv_event_t *e)
{
    auto *view =
        static_cast<FailSubview *>(
            lv_event_get_user_data(e));

    if (!view)
    {
        Log::warn(TAG, "Back callback received null view");
        return;
    }

    Log::debug(TAG, "Back button clicked");

    if (view->eventCallback)
        view->eventCallback("back");
}

void FailSubview::setMessage(const char *title, const char *message)
{
    Label::setText(titleLabel, title);
    Label::setText(messageLabel, message);
}
