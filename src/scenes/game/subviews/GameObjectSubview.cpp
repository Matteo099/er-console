#include "systems/ui/localization/I18n.h"
#include "GameObjectSubview.h"

#include "systems/ui/components/Button.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Align.h"
#include "systems/ui/components/ImageView.h"
#include "systems/ui/components/Container.h"
#include "log/Logger.h"

namespace {
constexpr const char* TAG = "GameObjectSubview";
}

void GameObjectSubview::create(lv_obj_t* parent) {
    Log::info(TAG, "Creating game object subview");

    root = Container::create(parent, LV_PCT(100), LV_PCT(100));

    auto layout = SceneLayout::create(root);

    header = layout.top;
    body = layout.body;
    footer = layout.footer;

    titleLabel = Label::create(header, "", UITheme::heading(), LV_TEXT_ALIGN_CENTER, LV_PCT(100));

    Align::setPosition(titleLabel, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_long_mode(titleLabel, LV_LABEL_LONG_DOT);
    lv_obj_set_height(titleLabel, UITheme::heading()->line_height);

    content = lv_obj_create(layout.body);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(content, 6, 0);
    lv_obj_set_style_pad_row(content, 8, 0);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    auto* footerRow = controls = lv_obj_create(footer);
    lv_obj_remove_style_all(footerRow);
    lv_obj_set_size(footerRow, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(footerRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footerRow, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    prevBtn = Button::create(footerRow, nullptr, onPrev, this, 44, 44, LV_ALIGN_CENTER, 0, 0,
                             LV_SYMBOL_LEFT);

    closeBtn = Button::create(footerRow, nullptr, onClose, this, 44, 44, LV_ALIGN_CENTER, 0, 0,
                              LV_SYMBOL_CLOSE);

    nextBtn = Button::create(footerRow, nullptr, onNext, this, 44, 44, LV_ALIGN_CENTER, 0, 0,
                             LV_SYMBOL_RIGHT);

    pageLabel = lv_label_create(footerRow);

    rebuild();
    hide();

    Log::info(TAG, "Game object subview created");
}

void GameObjectSubview::rebuild() {
    if (!content) {
        Log::warn(TAG, "rebuild() called with null content");
        return;
    }

    lv_obj_clean(content);

    if (objects.empty()) {
        Log::debug(TAG, "No objects to display");

        lv_label_set_text(titleLabel, tr(Text::Objects));
        lv_label_set_text(pageLabel, "0 / 0");

        lv_obj_add_state(prevBtn, LV_STATE_DISABLED);
        lv_obj_add_state(nextBtn, LV_STATE_DISABLED);

        auto* label = lv_label_create(content);
        lv_label_set_text(label, tr(Text::ObjectsHint));
        lv_obj_set_width(label, LV_PCT(100));
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(label);

        return;
    }

    lv_obj_set_state(prevBtn, LV_STATE_DISABLED, objects.size() < 2);
    lv_obj_set_state(nextBtn, LV_STATE_DISABLED, objects.size() < 2);

    auto& object = objects[currentIndex];

    Log::debug(TAG, "Displaying object %lu/%lu: %s", static_cast<unsigned long>(currentIndex + 1),
               static_cast<unsigned long>(objects.size()), object.title.c_str());

    lv_label_set_text(titleLabel, object.title.c_str());

    for (auto& block : object.content) {
        switch (block.type) {
            case ContentType::Text: {
                auto label = lv_label_create(content);
                lv_label_set_text(label, block.text.c_str());
                lv_obj_set_width(label, LV_PCT(100));
                lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
                lv_obj_set_style_text_font(label, UITheme::font(&lv_font_montserrat_14), 0);
                break;
            }

            case ContentType::Image: {
                lv_obj_t* img =
                    ImageView::create(content, block.imagePath.c_str(), LV_ALIGN_CENTER);

                if (!img) {
                    Log::warn(TAG, "Failed to create image: %s", block.imagePath.c_str());
                    break;
                }

                lv_obj_update_layout(img);
                const int32_t naturalWidth = lv_obj_get_width(img);
                const int32_t naturalHeight = lv_obj_get_height(img);
                const int32_t available = lv_obj_get_content_width(content);
                if (naturalWidth > 0 && available > 0) {
                    lv_obj_set_width(img,
                                     block.width == LV_SIZE_CONTENT
                                         ? (naturalWidth < available ? naturalWidth : available)
                                         : block.width);
                    lv_obj_update_layout(img);
                    lv_obj_set_height(img,
                                      block.height == LV_SIZE_CONTENT
                                          ? naturalHeight * lv_obj_get_width(img) / naturalWidth
                                          : block.height);
                    lv_image_set_inner_align(img, LV_IMAGE_ALIGN_CONTAIN);
                }

                break;
            }
        }
    }

    char page[32];
    snprintf(page, sizeof(page), "%lu / %lu", static_cast<unsigned long>(currentIndex + 1),
             static_cast<unsigned long>(objects.size()));

    lv_label_set_text(pageLabel, page);
}

void GameObjectSubview::setObjects(const std::vector<ObjectData>& data) {
    Log::info(TAG, "Loaded %u game objects", static_cast<unsigned>(data.size()));

    objects = data;
    currentIndex = 0;

    rebuild();
}

void GameObjectSubview::setCurrent(uint32_t index) {
    if (objects.empty()) {
        Log::warn(TAG, "setCurrent() called with no objects");
        currentIndex = 0;
        return;
    }

    currentIndex = index % objects.size();

    Log::debug(TAG, "Current object set to %lu", static_cast<unsigned long>(currentIndex + 1));

    rebuild();
    resetScroll();
}

void GameObjectSubview::onPrev(lv_event_t* e) {
    auto* self = static_cast<GameObjectSubview*>(lv_event_get_user_data(e));

    if (!self) {
        Log::warn(TAG, "Previous callback received null view");
        return;
    }

    if (self->objects.empty())
        return;

    if (self->currentIndex == 0)
        self->currentIndex = self->objects.size() - 1;
    else
        self->currentIndex--;

    Log::debug(TAG, "Previous button clicked");

    self->rebuild();

    if (self->callback)
        self->callback("prev");
}

void GameObjectSubview::onNext(lv_event_t* e) {
    auto* self = static_cast<GameObjectSubview*>(lv_event_get_user_data(e));

    if (!self) {
        Log::warn(TAG, "Next callback received null view");
        return;
    }

    if (self->objects.empty())
        return;

    self->currentIndex = (self->currentIndex + 1) % self->objects.size();

    Log::debug(TAG, "Next button clicked");

    self->rebuild();

    if (self->callback)
        self->callback("next");
}

void GameObjectSubview::onClose(lv_event_t* e) {
    auto* self = static_cast<GameObjectSubview*>(lv_event_get_user_data(e));

    if (!self) {
        Log::warn(TAG, "Close callback received null view");
        return;
    }

    Log::debug(TAG, "Close button clicked");

    if (self->callback)
        self->callback("back_main");
}

void GameObjectSubview::destroy() {
    Log::info(TAG, "Destroying game object subview");

    root = nullptr;
    header = nullptr;
    body = nullptr;
    footer = nullptr;
    controls = nullptr;
    titleLabel = nullptr;
    content = nullptr;
    prevBtn = nullptr;
    nextBtn = nullptr;
    closeBtn = nullptr;
    pageLabel = nullptr;

    objects.clear();
    currentIndex = 0;

    Log::info(TAG, "Game object subview destroyed");
}

void GameObjectSubview::show() {
    Log::debug(TAG, "Showing game object subview");

    if (root)
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void GameObjectSubview::hide() {
    Log::debug(TAG, "Hiding game object subview");

    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void GameObjectSubview::update() {
}

void GameObjectSubview::setEventCallback(EventCallback cb) {
    Log::debug(TAG, "Event callback registered");
    callback = std::move(cb);
}

void GameObjectSubview::next() {
    if (objects.empty())
        return;

    currentIndex = (currentIndex + 1) % objects.size();

    Log::debug(TAG, "next() -> object %lu", static_cast<unsigned long>(currentIndex + 1));

    rebuild();
    resetScroll();
}

void GameObjectSubview::previous() {
    if (objects.empty())
        return;

    if (currentIndex == 0)
        currentIndex = objects.size() - 1;
    else
        currentIndex--;

    Log::debug(TAG, "previous() -> object %lu", static_cast<unsigned long>(currentIndex + 1));

    rebuild();
    resetScroll();
}

void GameObjectSubview::resetScroll() {
    if (!content) {
        Log::warn(TAG, "resetScroll() called with null content");
        return;
    }

    lv_obj_scroll_to(content, 0, 0, LV_ANIM_OFF);
}

const GameObjectSubview::ObjectData* GameObjectSubview::getCurrent() const {
    if (objects.empty())
        return nullptr;

    return &objects[currentIndex];
}
