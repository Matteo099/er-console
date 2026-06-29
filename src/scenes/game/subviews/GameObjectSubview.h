#pragma once

#include <lvgl.h>
#include <string>
#include <vector>
#include "../GameSubview.h"

class GameObjectSubview
{
public:
    enum class ContentType
    {
        Text,
        Image
    };

    struct ContentBlock
    {
        ContentType type;

        // Text
        std::string text;

        // Image path
        std::string imagePath;

        lv_coord_t width = LV_SIZE_CONTENT;
        lv_coord_t height = LV_SIZE_CONTENT;

        static ContentBlock textBlock(
            const std::string &value)
        {
            ContentBlock b;
            b.type = ContentType::Text;
            b.text = value;
            return b;
        }

        static ContentBlock imageBlock(
            const std::string &path,
            lv_coord_t w = LV_SIZE_CONTENT,
            lv_coord_t h = LV_SIZE_CONTENT)
        {
            ContentBlock b;
            b.type = ContentType::Image;
            b.imagePath = path;
            b.width = w;
            b.height = h;
            return b;
        }
    };

    struct ObjectData
    {
        std::string title;
        std::vector<ContentBlock> content;
    };

    void create(lv_obj_t *parent);
    void destroy();
    void update();
    void show();
    void hide();
    void setEventCallback(EventCallback cb);
    void setObjects(const std::vector<ObjectData> &data);
    void setCurrent(uint32_t index);
    void next();
    void previous();
    const ObjectData *getCurrent() const;

private:
    lv_obj_t *root = nullptr;
    lv_obj_t *header = nullptr;
    lv_obj_t *body = nullptr;
    lv_obj_t *footer = nullptr;
    lv_obj_t *titleLabel = nullptr;
    lv_obj_t *content = nullptr;
    lv_obj_t *prevBtn = nullptr;
    lv_obj_t *nextBtn = nullptr;
    lv_obj_t *closeBtn = nullptr;
    lv_obj_t *pageLabel = nullptr;

    std::vector<ObjectData> objects;
    uint32_t currentIndex = 0;
    EventCallback callback;

    void rebuild();
    void resetScroll();
    static void onPrev(lv_event_t *e);
    static void onNext(lv_event_t *e);
    static void onClose(lv_event_t *e);
};