#include "LoadingSubview.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "log/Logger.h"

namespace
{
    constexpr const char *TAG = "LoadingSubview";
}

void LoadingSubview::create(lv_obj_t *parent)
{
    Log::info(TAG, "Creating loading subview");

    root = Container::create(
        parent,
        LV_PCT(100),
        LV_PCT(100));

    spinner = lv_spinner_create(root);

    lv_obj_set_size(spinner, 48, 48);

    Align::setPosition(
        spinner,
        LV_ALIGN_CENTER,
        0,
        -25);

    messageLabel =
        Label::create(
            root,
            "Loading...",
            &lv_font_roboto_mono_16,
            LV_TEXT_ALIGN_CENTER,
            220);

    Align::setPosition(
        messageLabel,
        LV_ALIGN_CENTER,
        0,
        45);

    hide();

    Log::info(TAG, "Loading subview created");
}

void LoadingSubview::destroy()
{
    Log::info(TAG, "Destroying loading subview");

    root = nullptr;
    spinner = nullptr;
    messageLabel = nullptr;

    Log::info(TAG, "Loading subview destroyed");
}

void LoadingSubview::show()
{
    Log::debug(TAG, "Showing loading subview");

    if (root)
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void LoadingSubview::hide()
{
    Log::debug(TAG, "Hiding loading subview");

    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void LoadingSubview::setMessage(const std::string &message)
{
    if (!messageLabel)
    {
        Log::warn(TAG, "setMessage() called before messageLabel was created");
        return;
    }

    Log::debug(TAG, "Loading message: %s", message.c_str());

    lv_label_set_text(
        messageLabel,
        message.c_str());
}