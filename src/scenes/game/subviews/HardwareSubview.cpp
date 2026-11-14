#include "HardwareSubview.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/localization/I18n.h"
#include <algorithm>

void HardwareSubview::create(lv_obj_t* parent, EventCallback value) {
    callback = std::move(value);
    root = Container::create(parent, LV_PCT(100), LV_PCT(100));
    auto layout = SceneLayout::create(root, 28, 48, false);
    title = Label::create(layout.top, "", UITheme::heading(18), LV_TEXT_ALIGN_CENTER, LV_PCT(100));
    UITheme::danger(title);
    lv_obj_center(title);
    Container::setFlex(layout.body);
    Container::setScrollable(layout.body);
    lv_obj_set_style_pad_row(layout.body, 8, 0);
    details =
        Label::create(layout.body, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    hint = Label::create(layout.body, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(hint);
    Container::setFlex(layout.footer, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_SPACE_EVENLY,
                       LV_FLEX_ALIGN_CENTER);
    retry = Button::create(layout.footer, "", onAction, this, 132, 44, LV_ALIGN_CENTER, 0, 0,
                           LV_SYMBOL_REFRESH);
    back = Button::create(layout.footer, "", onAction, this, 140, 44, LV_ALIGN_CENTER, 0, 0,
                          LV_SYMBOL_LEFT);
    UITheme::secondary(back);
    translate();
    hide();
}

void HardwareSubview::translate() {
    if (!root)
        return;
    Label::setText(title, tr(interrupted ? Text::HardwareDisconnected : Text::HardwareCannotStart));
    std::string text;
    bool configuration = false;
    for (const auto& issue : report.issues) {
        Text key = Text::HardwareInitializationError;
        switch (issue.kind) {
            case HardwareIssueKind::Configuration:
                key = Text::HardwareConfigurationError;
                configuration = true;
                break;
            case HardwareIssueKind::Unsupported:
                key = Text::HardwareUnsupported;
                configuration = true;
                break;
            case HardwareIssueKind::MissingI2C:
            case HardwareIssueKind::MissingRFID:
                key = Text::HardwareDeviceMissing;
                break;
            case HardwareIssueKind::MissingIdentity:
                key = Text::HardwareIdentityMissing;
                break;
            case HardwareIssueKind::WrongRoom:
                key = Text::HardwareWrongRoom;
                break;
            default:
                break;
        }
        char line[256];
        lv_snprintf(line, sizeof(line), tr(key), issue.component.c_str());
        if (!text.empty())
            text += "\n\n";
        text += line;
    }
    Label::setText(details, text.c_str());
    std::string guidance =
        tr(configuration ? Text::HardwareConfigurationHelp : Text::HardwareConnectionHelp);
    if (interrupted)
        guidance += std::string("\n\n") + tr(Text::HardwareRestartHelp);
    Label::setText(hint, guidance.c_str());
    Label::setText(lv_obj_get_child(retry, lv_obj_get_child_count(retry) - 1), tr(Text::Retry));
    Label::setText(lv_obj_get_child(back, lv_obj_get_child_count(back) - 1), tr(Text::Back));
}

void HardwareSubview::show(const HardwareStatus& value, bool wasInterrupted) {
    report = value;
    interrupted = wasInterrupted;
    translate();
    lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
    lv_obj_scroll_to_y(lv_obj_get_parent(details), 0, LV_ANIM_OFF);
}

void HardwareSubview::hide() {
    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void HardwareSubview::destroy() {
    root = title = details = hint = retry = back = nullptr;
    callback = nullptr;
}

void HardwareSubview::onAction(lv_event_t* event) {
    auto* self = static_cast<HardwareSubview*>(lv_event_get_user_data(event));
    if (self->callback)
        self->callback(lv_event_get_target(event) == self->retry ? "retry" : "back");
}
