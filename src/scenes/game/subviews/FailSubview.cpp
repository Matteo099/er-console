#include "FailSubview.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "log/Logger.h"

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

    titleLabel =
        Label::create(
            root,
            "Tempo scaduto",
            &lv_font_roboto_mono_28,
            LV_TEXT_ALIGN_CENTER,
            240);

    Label::setTextColor(
        titleLabel,
        lv_color_hex(0xA44848));

    Align::setPosition(
        titleLabel,
        LV_ALIGN_TOP_MID,
        0,
        30);

    retryBtn =
        Button::create(
            root,
            "Riprova",
            onRetry,
            this,
            140,
            44,
            LV_ALIGN_CENTER,
            0,
            -10,
            LV_SYMBOL_REFRESH);

    backBtn =
        Button::create(
            root,
            "Indietro",
            onBack,
            this,
            140,
            44,
            LV_ALIGN_CENTER,
            0,
            50,
            LV_SYMBOL_LEFT);

    hide();

    Log::info(TAG, "Fail subview created");
}

void FailSubview::destroy()
{
    Log::info(TAG, "Destroying fail subview");

    root = nullptr;
    titleLabel = nullptr;
    retryBtn = nullptr;
    backBtn = nullptr;

    Log::info(TAG, "Fail subview destroyed");
}

void FailSubview::show()
{
    Log::debug(TAG, "Showing fail subview");

    if (root)
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
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
