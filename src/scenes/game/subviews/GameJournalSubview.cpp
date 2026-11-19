#include "GameJournalSubview.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/localization/I18n.h"

void GameJournalSubview::create(lv_obj_t* parent, EventCallback callback,
                                std::function<void(bool, size_t)> open) {
    back = std::move(callback);
    selected = std::move(open);
    root = Container::create(parent, LV_PCT(100), LV_PCT(100));
    auto layout = SceneLayout::create(root, 48, 48, true);
    body = layout.body;
    controls = layout.footer;
    Container::setFlex(layout.top, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_SPACE_EVENLY,
                       LV_FLEX_ALIGN_CENTER);
    objectButton = Button::create(layout.top, "", onTab, this, LV_PCT(48), 44);
    hintButton = Button::create(layout.top, "", onTab, this, LV_PCT(48), 44);
    Container::setFlex(body);
    lv_obj_set_style_pad_row(body, 6, 0);
    Container::setFlex(controls, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_SPACE_EVENLY,
                       LV_FLEX_ALIGN_CENTER);
    backButton =
        Button::create(controls, "", onBack, this, 200, 44, LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_LEFT);
    UITheme::secondary(backButton);
    hide();
}

void GameJournalSubview::setLists(const std::vector<GameObjectSubview::ObjectData>& objects,
                                  const std::vector<GameObjectSubview::ObjectData>& hints) {
    objectTitles.clear();
    hintTitles.clear();
    for (const auto& entry : objects)
        objectTitles.push_back(entry.title);
    for (const auto& entry : hints)
        hintTitles.push_back(entry.title);
    if (root && lv_obj_is_visible(root))
        rebuild();
}

void GameJournalSubview::rebuild() {
    if (!body)
        return;
    lv_obj_clean(body);
    for (auto* tab : {objectButton, hintButton}) {
        UITheme::secondary(tab);
        lv_obj_set_style_bg_color(tab, lv_color_hex(UITheme::palette().strong), LV_STATE_CHECKED);
        lv_obj_set_style_text_color(tab, lv_color_white(), LV_STATE_CHECKED);
    }
    char caption[80];
    lv_snprintf(caption, sizeof(caption), "%s (%u)", tr(Text::Objects),
                unsigned(objectTitles.size()));
    Label::setText(lv_obj_get_child(objectButton, 0), caption);
    lv_snprintf(caption, sizeof(caption), "%s (%u)", tr(Text::Hints), unsigned(hintTitles.size()));
    Label::setText(lv_obj_get_child(hintButton, 0), caption);
    lv_obj_set_state(objectButton, LV_STATE_CHECKED, !hintsTab);
    lv_obj_set_state(hintButton, LV_STATE_CHECKED, hintsTab);
    lv_obj_set_style_text_color(lv_obj_get_child(objectButton, 0),
                                hintsTab ? lv_color_hex(UITheme::palette().ink) : lv_color_white(),
                                0);
    lv_obj_set_style_text_color(lv_obj_get_child(hintButton, 0),
                                hintsTab ? lv_color_white() : lv_color_hex(UITheme::palette().ink),
                                0);
    Label::setText(lv_obj_get_child(backButton, lv_obj_get_child_count(backButton) - 1),
                   tr(Text::Back));
    const auto& entries = hintsTab ? hintTitles : objectTitles;
    if (entries.empty()) {
        auto* empty = Label::create(body, tr(hintsTab ? Text::HintsEmpty : Text::ObjectsHint),
                                    &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER, LV_PCT(100));
        UITheme::muted(empty);
    }
    for (size_t i = 0; i < entries.size(); ++i) {
        auto* row =
            Button::create(body, entries[i].c_str(), onEntry, this, LV_PCT(100), 48,
                           LV_ALIGN_CENTER, 0, 0, hintsTab ? LV_SYMBOL_EYE_OPEN : LV_SYMBOL_FILE);
        lv_obj_set_user_data(row, reinterpret_cast<void*>(i + 1));
        auto* label = lv_obj_get_child(row, lv_obj_get_child_count(row) - 1);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_font(label, UITheme::font(&lv_font_montserrat_14), 0);
        lv_obj_set_width(label, 240);
        lv_obj_set_height(label, 32);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
    }
    lv_obj_scroll_to_y(body, 0, LV_ANIM_OFF);
}

void GameJournalSubview::translate() {
    if (root && lv_obj_is_visible(root))
        rebuild();
}

void GameJournalSubview::show(bool hints) {
    hintsTab = hints;
    rebuild();
    lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void GameJournalSubview::hide() {
    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void GameJournalSubview::destroy() {
    root = body = controls = objectButton = hintButton = backButton = nullptr;
    objectTitles.clear();
    hintTitles.clear();
    back = nullptr;
    selected = nullptr;
}

void GameJournalSubview::onTab(lv_event_t* event) {
    auto* self = static_cast<GameJournalSubview*>(lv_event_get_user_data(event));
    self->hintsTab = lv_event_get_target(event) == self->hintButton;
    self->rebuild();
}

void GameJournalSubview::onEntry(lv_event_t* event) {
    auto* self = static_cast<GameJournalSubview*>(lv_event_get_user_data(event));
    const size_t index =
        reinterpret_cast<uintptr_t>(lv_obj_get_user_data(lv_event_get_target_obj(event))) - 1;
    if (self->selected)
        self->selected(self->hintsTab, index);
}

void GameJournalSubview::onBack(lv_event_t* event) {
    auto* self = static_cast<GameJournalSubview*>(lv_event_get_user_data(event));
    if (self->back)
        self->back("journal_back");
}
