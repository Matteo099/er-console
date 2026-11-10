#pragma once
#include <functional>
#include "systems/network/WifiScan.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Button.h"
#include "systems/ui/components/SceneLayout.h"

// Native LVGL shapes keep signal/security icons independent of font glyphs.
class WifiNetworkPicker
{
public:
    void create(lv_obj_t *parent, std::function<void(WifiScanEntry)> selected, std::function<void()> refresh)
    {
        chosen = std::move(selected); scanRequested = std::move(refresh);
        root = Container::create(parent, LV_PCT(100), LV_PCT(100), LV_ALIGN_TOP_LEFT);
        lv_obj_add_flag(root, LV_OBJ_FLAG_FLOATING); UITheme::screen(root);
        auto layout = SceneLayout::create(root);
        for (auto *row : {layout.top, layout.body, layout.footer})
        {
            UITheme::screen(row); lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
        }
        BrandHeader::create(layout.top, 100, tr(Text::WifiNetworks));
        auto *column = Container::create(layout.body, LV_PCT(100), LV_SIZE_CONTENT, LV_ALIGN_TOP_MID);
        Container::setFlex(column); lv_obj_set_style_pad_row(column, 6, 0);
        message = Label::create(column, tr(Text::ChooseNetwork), &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
        UITheme::muted(message);
        rows = Container::create(column, LV_PCT(100), LV_SIZE_CONTENT);
        Container::setFlex(rows); lv_obj_set_style_pad_row(rows, 6, 0);
        Container::setFlex(layout.footer, LV_FLEX_FLOW_ROW, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(layout.footer, 8, 0);
        auto *back = Button::create(layout.footer, tr(Text::Back), onBack, this, 132, 44);
        UITheme::secondary(back);
        refreshButton = Button::create(layout.footer, tr(Text::Refresh), onRefresh, this, 132, 44);
        hide();
    }
    void show() { lv_obj_remove_flag(root,LV_OBJ_FLAG_HIDDEN); lv_obj_move_foreground(root); }
    void hide() { lv_obj_add_flag(root,LV_OBJ_FLAG_HIDDEN); }
    void clear() { root = rows = message = refreshButton = nullptr; buttons.clear(); networks.clear(); chosen = nullptr; scanRequested = nullptr; }
    void setSelected(const std::string &ssid, bool secure) { selectedSsid = ssid; selectedSecure = secure; }
    void setSnapshot(const WifiScanSnapshot &snapshot)
    {
        networks = snapshot.networks;
        Text hint = Text::ChooseNetwork;
        switch (snapshot.state)
        {
        case WifiScanState::Disabled: hint = Text::WifiDisabled; break;
        case WifiScanState::Scanning: hint = Text::ScanningNetworks; break;
        case WifiScanState::Error: hint = Text::ScanFailed; break;
        case WifiScanState::Busy: hint = Text::ScanBusy; break;
        case WifiScanState::Ready: hint = networks.empty() ? Text::NoNetworks : Text::ChooseNetwork; break;
        default: break;
        }
        Label::setText(message,tr(hint));
        lv_obj_set_flag(message,LV_OBJ_FLAG_HIDDEN,snapshot.state == WifiScanState::Ready && !networks.empty());
        lv_obj_set_state(refreshButton,LV_STATE_DISABLED,snapshot.state == WifiScanState::Disabled || snapshot.state == WifiScanState::Scanning);
        lv_obj_clean(rows); buttons.clear();
        for (const auto &network : networks)
        {
            auto *row = Button::create(rows,nullptr,onSelected,this,LV_PCT(100),56);
            UITheme::secondary(row); lv_obj_set_style_pad_hor(row,8,0);
            lv_obj_set_state(row,LV_STATE_CHECKED,network.ssid == selectedSsid && network.secure == selectedSecure);
            signalIcon(row,network.signal);
            auto *text = Container::create(row,0,LV_SIZE_CONTENT);
            lv_obj_remove_flag(text,LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_flex_grow(text,1); Container::setFlex(text,LV_FLEX_FLOW_COLUMN,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_START);
            auto *title = Label::create(text,network.ssid.c_str(),&lv_font_montserrat_14,LV_TEXT_ALIGN_LEFT,LV_PCT(100));
            lv_obj_set_height(title,lv_font_montserrat_14.line_height); lv_label_set_long_mode(title,LV_LABEL_LONG_DOT);
            char detail[64]; lv_snprintf(detail,sizeof(detail),"%s   %d dBm",tr(network.secure ? Text::NetworkProtected : Text::NetworkOpen),network.signal);
            auto *subtitle = Label::create(text,detail,&lv_font_montserrat_12); UITheme::muted(subtitle);
            lockIcon(row,network.secure);
            buttons.push_back(row);
        }
    }
private:
    static void signalIcon(lv_obj_t *parent, int rssi)
    {
        auto *icon = Container::create(parent,22,20);
        lv_obj_remove_flag(icon,LV_OBJ_FLAG_CLICKABLE);
        const unsigned count = WifiScan::signalBars(rssi);
        for (unsigned i=0;i<4;++i)
        {
            auto *bar = Container::create(icon,3,4+i*4,LV_ALIGN_BOTTOM_LEFT,i*5,0);
            lv_obj_remove_flag(bar,LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_bg_opa(bar,LV_OPA_COVER,0);
            lv_obj_set_style_radius(bar,1,0);
            lv_obj_set_style_bg_color(bar,lv_color_hex(i<count ? UITheme::palette().strong : UITheme::palette().border),0);
        }
    }
    static void lockIcon(lv_obj_t *parent, bool secured)
    {
        auto *icon = Container::create(parent,20,22);
        lv_obj_remove_flag(icon,LV_OBJ_FLAG_CLICKABLE);
        auto *shackle = Container::create(icon,10,11,LV_ALIGN_TOP_LEFT,secured ? 4 : 9,1);
        lv_obj_remove_flag(shackle,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_radius(shackle,5,0); lv_obj_set_style_border_width(shackle,2,0);
        lv_obj_set_style_border_color(shackle,lv_color_hex(UITheme::palette().strong),0);
        auto *body = Container::create(icon,16,11,LV_ALIGN_TOP_LEFT,1,9);
        lv_obj_remove_flag(body,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(body,LV_OPA_COVER,0); lv_obj_set_style_radius(body,2,0);
        lv_obj_set_style_bg_color(body,lv_color_hex(UITheme::palette().strong),0);
    }
    static void onBack(lv_event_t *e) { static_cast<WifiNetworkPicker *>(lv_event_get_user_data(e))->hide(); }
    static void onRefresh(lv_event_t *e)
    {
        auto *self = static_cast<WifiNetworkPicker *>(lv_event_get_user_data(e));
        if (self->scanRequested) self->scanRequested();
    }
    static void onSelected(lv_event_t *e)
    {
        auto *self = static_cast<WifiNetworkPicker *>(lv_event_get_user_data(e));
        for (unsigned i=0;i<self->buttons.size();++i)
            if (self->buttons[i] == lv_event_get_target(e))
            {
                const auto entry = self->networks[i]; self->hide();
                if (self->chosen) self->chosen(entry);
                return;
            }
    }
    lv_obj_t *root = nullptr, *rows = nullptr, *message = nullptr, *refreshButton = nullptr;
    std::vector<WifiScanEntry> networks;
    std::vector<lv_obj_t *> buttons;
    std::string selectedSsid;
    bool selectedSecure = true;
    std::function<void(WifiScanEntry)> chosen;
    std::function<void()> scanRequested;
};
