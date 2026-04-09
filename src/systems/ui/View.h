#pragma once

#include <lvgl.h>
#include "UIObjectTracker.h"
#include "UIEventSystem.h"

class View : public UIEventEmitter
{
public:
    virtual ~View() = default;

    virtual void create() = 0;
    virtual void update() = 0;
    virtual void destroy() = 0;

    virtual lv_obj_t* getScreen() = 0;

    /**
     * @brief Get the object tracker for this view
     * Use this to track dynamically created objects that need cleanup
     */
    UIObjectTracker& getObjectTracker()
    {
        return objectTracker;
    }

    // UIEventEmitter implementation
    void setEventHandler(UIEventHandler* handler) override
    {
        eventHandler = handler;
    }

    void emitEvent(const UIEvent& event) override
    {
        if (eventHandler)
        {
            eventHandler->handleUIEvent(event);
        }
    }

protected:
    // Optional object tracking for complex views with dynamic objects
    // Views can track dynamically created objects via getObjectTracker().track()
    UIObjectTracker objectTracker;

    // Event handler (typically the scene)
    UIEventHandler* eventHandler = nullptr;
};

