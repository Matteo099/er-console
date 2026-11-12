#include "LocalizedText.h"
#include <algorithm>
#include <vector>

namespace {
struct Binding {
    lv_obj_t* label;
    Text key;
    std::string prefix;
};

std::vector<Binding*> bindings;

void deleted(lv_event_t* event) {
    auto* binding = static_cast<Binding*>(lv_event_get_user_data(event));
    bindings.erase(std::remove(bindings.begin(), bindings.end(), binding), bindings.end());
    delete binding;
}
}  // namespace

void LocalizedText::bind(lv_obj_t* label, const char* text) {
    bind(label, I18n::identify(text));
}

void LocalizedText::bind(lv_obj_t* label, Text key, const char* prefix) {
    auto found = std::find_if(bindings.begin(), bindings.end(), [label](Binding* b) {
        return b->label == label;
    });
    if (found != bindings.end()) {
        (*found)->key = key;
        (*found)->prefix = prefix;
        return;
    }
    if (key == Text::None)
        return;
    auto* binding = new Binding{label, key, prefix};
    bindings.push_back(binding);
    lv_obj_add_event_cb(label, deleted, LV_EVENT_DELETE, binding);
}

void LocalizedText::refresh() {
    for (const auto* binding : bindings)
        if (binding->key != Text::None)
            lv_label_set_text(binding->label, (binding->prefix + tr(binding->key)).c_str());
}
