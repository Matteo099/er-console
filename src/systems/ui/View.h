#pragma once

#include <lvgl.h>
#include "UIEventSystem.h"

class View : public UIEventEmitter {
  public:
    virtual ~View() = default;

    virtual void create() = 0;
    virtual void update() = 0;
    virtual void destroy() = 0;

    virtual lv_obj_t* getScreen() = 0;

    // UIEventEmitter implementation
    void setEventHandler(UIEventHandler* handler) override {
        eventHandler = handler;
    }

    void emitEvent(const UIEvent& event) override {
        if (eventHandler) {
            eventHandler->handleUIEvent(event);
        }
    }

  protected:
    // Event handler (typically the scene)
    UIEventHandler* eventHandler = nullptr;
};
