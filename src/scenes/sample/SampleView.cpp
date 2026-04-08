#include "SampleView.h"
#include "log/Logger.h"

#include "systems/ui/components/BrandHeader.h"

void SampleView::create()
{
    // --- Screen ---
    screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    btn = lv_btn_create(screen);
    lv_obj_center(btn);
    lv_obj_add_event_cb(btn, onButtonClicked, LV_EVENT_CLICKED, this);

    label = lv_label_create(btn);
    lv_label_set_text(label, "Play Sound");
    lv_obj_center(label);

    lv_scr_load(screen);
}

void SampleView::update()
{
    if (!screen)
        return;
}

void SampleView::destroy()
{
    if (screen)
    {
        lv_obj_del(screen);
    }

    // Reset all pointers
    screen = nullptr;
    btn = nullptr;
    label = nullptr;
}

void SampleView::onButtonClicked(lv_event_t *e)
{
    auto *view =
        static_cast<SampleView *>(lv_event_get_user_data(e));

    if (!view)
        return;

    view->playSound = true;
}

bool SampleView::consumeAction()
{
    bool a = playSound;
    playSound = false;
    return a;
}
