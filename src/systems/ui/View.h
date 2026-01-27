#pragma once

#include <lvgl.h>

class View {
public:
    virtual ~View() = default;

    virtual void create() = 0;
    virtual void update() = 0;
    virtual void destroy() = 0;

    virtual lv_obj_t* getScreen() = 0;
};
