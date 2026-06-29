#include "LoadingSubview.h"

#include "systems/ui/components/Align.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"

void LoadingSubview::create(lv_obj_t *parent)
{
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
}

void LoadingSubview::destroy()
{
    root = nullptr;
    spinner = nullptr;
    messageLabel = nullptr;
}

void LoadingSubview::show()
{
    if (root)
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void LoadingSubview::hide()
{
    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void LoadingSubview::setMessage(const std::string &message)
{
    if (!messageLabel)
        return;

    lv_label_set_text(
        messageLabel,
        message.c_str());
}