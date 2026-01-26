#include "UIManager.h"

#include "core/SystemEventBus.h"
#include "os/BootEvent.h"
#include "scenes/boot/BootView.h"

void UIManager::init()
{
    lvgl = new LVGLSystem();
    lvgl->init();
    XPT2046Touch touch(TOUCH_CS_PIN);
    touch.init();
    lvgl_register_touch(&touch);

    BootView bootView;
    bootView.create();
}

void UIManager::update()
{
    BootEvent evt;
    while (xQueueReceive(SystemEventBus::bootQueue, &evt, 0) == pdTRUE)
    {
        bootView.update(evt.progress, evt.label);
    }

    lv_timer_handler();
}