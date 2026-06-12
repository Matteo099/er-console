#include "SampleView.h"
#include "log/Logger.h"
#include "config/Assets.h"

#include "systems/ui/components/Button.h"
#include "systems/ui/components/Screen.h"

void SampleView::create()
{
    screen = Screen::create();

    btn = Button::create(screen, "Play Sound", onButtonClicked, this);
}

void SampleView::update()
{
    if (!screen)
        return;
}

void SampleView::destroy()
{
    screen = nullptr;

    // Nullify remaining pointers
    btn = nullptr;
}

void SampleView::onButtonClicked(lv_event_t *e)
{
    auto *view = static_cast<SampleView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    // Emit event instead of setting flag
    PlaySoundEvent event(AUDIO_CLICK_FILE);
    view->emitEvent(event);
}
