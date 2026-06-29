#include "GameObjectSubview.h"

#include "systems/ui/components/Button.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Align.h"
#include "systems/ui/components/ImageView.h"

void GameObjectSubview::create(lv_obj_t *parent)
{
    auto layout = SceneLayout::create(parent);
    root = parent;
    header = layout.top;
    body = layout.body;
    footer = layout.footer;
    titleLabel = Label::create(header, "", &lv_font_roboto_mono_20, LV_TEXT_ALIGN_CENTER, LV_PCT(100));
    Align::setPosition(titleLabel, LV_ALIGN_CENTER, 0, 0);
    content = lv_obj_create(body);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_remove_style_all(content);
    lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(content, 10, 0);
    auto footerRow = lv_obj_create(footer);
    lv_obj_remove_style_all(footerRow);
    lv_obj_set_size(footerRow, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(footerRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footerRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    prevBtn = Button::create(footerRow, nullptr, onPrev, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_LEFT);
    closeBtn = Button::create(footerRow, nullptr, onClose, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_CLOSE);
    nextBtn = Button::create(footerRow, nullptr, onNext, this, 44, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_RIGHT);
    pageLabel = lv_label_create(footerRow);
    hide();
}

void GameObjectSubview::rebuild()
{
    if (!content)
        return;

    lv_obj_clean(content);
    if (objects.empty())
        return;

    auto &object = objects[currentIndex];
    lv_label_set_text(titleLabel, object.title.c_str());

    for (auto &block : object.content)
    {
        switch (block.type)
        {
        case ContentType::Text:
        {
            auto label = lv_label_create(content);
            lv_label_set_text(label, block.text.c_str());
            lv_obj_set_width(label, LV_PCT(100));
            lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
            lv_obj_set_style_text_font(label, &lv_font_roboto_mono_16, 0);
            break;
        }
        case ContentType::Image:
        {
            lv_obj_t *img = ImageView::create(content, block.imagePath.c_str(), LV_ALIGN_CENTER);
            if (!img)
                break;
            if (block.width != LV_SIZE_CONTENT || block.height != LV_SIZE_CONTENT)
            {
                lv_obj_set_size(img, block.width, block.height);
            }
            break;
        }
        }
    }
    char page[32];
    snprintf(page, sizeof(page), "%lu / %lu", currentIndex + 1, objects.size());
    lv_label_set_text(pageLabel, page);
}

void GameObjectSubview::setObjects(const std::vector<ObjectData> &data)
{
    objects = data;
    currentIndex = 0;
    rebuild();
}

void GameObjectSubview::setCurrent(
    uint32_t index)
{
    if (objects.empty())
    {
        currentIndex = 0;
        return;
    }

    currentIndex =
        index % objects.size();

    rebuild();

    resetScroll();
}

void GameObjectSubview::onPrev(lv_event_t *e)
{
    auto *self = static_cast<GameObjectSubview *>(lv_event_get_user_data(e));
    if (!self || self->objects.empty())
        return;
    if (self->currentIndex == 0)
        self->currentIndex = self->objects.size() - 1;
    else
        self->currentIndex--;
    self->rebuild();
    if (self->callback)
        self->callback("prev");
}

void GameObjectSubview::onNext(lv_event_t *e)
{
    auto *self = static_cast<GameObjectSubview *>(lv_event_get_user_data(e));
    if (!self || self->objects.empty())
        return;
    self->currentIndex = (self->currentIndex + 1) % self->objects.size();
    self->rebuild();
    if (self->callback)
        self->callback("next");
}

void GameObjectSubview::onClose(lv_event_t *e)
{
    auto *self =
        static_cast<GameObjectSubview *>(lv_event_get_user_data(e));

    if (!self)
        return;

    if (self->callback)
        self->callback("back_main");
}

void GameObjectSubview::destroy()
{
    root = nullptr;
    header = nullptr;
    body = nullptr;
    footer = nullptr;
    titleLabel = nullptr;
    content = nullptr;
    prevBtn = nullptr;
    nextBtn = nullptr;
    closeBtn = nullptr;
    pageLabel = nullptr;
    objects.clear();
    currentIndex = 0;
}

void GameObjectSubview::show()
{
    if (root)
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void GameObjectSubview::hide()
{
    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void GameObjectSubview::update() {}

void GameObjectSubview::setEventCallback(EventCallback cb) { callback = std::move(cb); }

void GameObjectSubview::next()
{
    if (objects.empty())
        return;
    currentIndex = (currentIndex + 1) % objects.size();
    rebuild();
    resetScroll();
}

void GameObjectSubview::previous()
{
    if (objects.empty())
        return;
    if (currentIndex == 0)
        currentIndex = objects.size() - 1;
    else
        currentIndex--;
    rebuild();
    resetScroll();
}

void GameObjectSubview::resetScroll()
{
    if (!content)
        return;
    lv_obj_scroll_to(content, 0, 0, LV_ANIM_OFF);
}
const GameObjectSubview::ObjectData *GameObjectSubview::getCurrent() const
{
    if (objects.empty())
        return nullptr;
    return &objects[currentIndex];
}