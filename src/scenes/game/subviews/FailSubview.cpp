#include "FailSubview.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"

void FailSubview::create(lv_obj_t* parent)
{
    root = Container::create(
        parent,
        LV_PCT(100),
        LV_PCT(100));

    titleLabel =
        Label::create(
            root,
            "YOU LOSE!",
            &lv_font_roboto_mono_28,
            LV_TEXT_ALIGN_CENTER,
            240);

    Label::setTextColor(
        titleLabel,
        lv_color_hex(0xFF0000));

    Align::setPosition(
        titleLabel,
        LV_ALIGN_TOP_MID,
        0,
        30);

    retryBtn =
        Button::create(
            root,
            "Retry",
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
            "Back",
            onBack,
            this,
            140,
            44,
            LV_ALIGN_CENTER,
            0,
            50,
            LV_SYMBOL_LEFT);

    hide();
}

void FailSubview::destroy()
{
    root = nullptr;
    titleLabel = nullptr;
    retryBtn = nullptr;
    backBtn = nullptr;
}

void FailSubview::show()
{
    if (root)
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void FailSubview::hide()
{
    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void FailSubview::setEventCallback(EventCallback callback)
{
    eventCallback = std::move(callback);
}

void FailSubview::onRetry(lv_event_t* e)
{
    auto* view =
        static_cast<FailSubview*>(
            lv_event_get_user_data(e));

    if (!view)
        return;

    if (view->eventCallback)
        view->eventCallback("retry");
}

void FailSubview::onBack(lv_event_t* e)
{
    auto* view =
        static_cast<FailSubview*>(
            lv_event_get_user_data(e));

    if (!view)
        return;

    if (view->eventCallback)
        view->eventCallback("back");
}