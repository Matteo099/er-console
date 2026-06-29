#pragma once

#include <lvgl.h>
#include <functional>

class FailSubview
{
public:
    using EventCallback = std::function<void(const char*)>;

    void create(lv_obj_t* parent);
    void destroy();

    void show();
    void hide();

    void setEventCallback(EventCallback callback);

private:
    static void onRetry(lv_event_t* e);
    static void onBack(lv_event_t* e);

    EventCallback eventCallback;

    lv_obj_t* root = nullptr;

    lv_obj_t* titleLabel = nullptr;
    lv_obj_t* retryBtn = nullptr;
    lv_obj_t* backBtn = nullptr;
};