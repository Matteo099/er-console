#pragma once

#include <lvgl.h>
#include <string>

class LoadingSubview
{
public:
    void create(lv_obj_t* parent);
    void destroy();

    void show();
    void hide();

    void setMessage(const std::string& message);

private:
    lv_obj_t* root = nullptr;

    lv_obj_t* spinner = nullptr;
    lv_obj_t* messageLabel = nullptr;
};