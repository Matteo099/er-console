#pragma once

#include <stdint.h>
#include "systems/ui/View.h"

class AboutView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; };

private:
    lv_obj_t *screen = nullptr;
};
