#pragma once
#include "BrandHeader.h"
#include "SceneLayout.h"
#include "Button.h"
#include "Container.h"
#include "Label.h"
#include <functional>
#include <cstdint>

class PlayerSelectorDialog
{
public:
    void create(lv_obj_t *parent,std::function<void(bool,uint16_t)> selected)
    {
        callback=std::move(selected);
        root=Container::create(parent,LV_PCT(100),LV_PCT(100),LV_ALIGN_TOP_LEFT);
        lv_obj_add_flag(root,LV_OBJ_FLAG_FLOATING); lv_obj_add_flag(root,LV_OBJ_FLAG_CLICKABLE);
        UITheme::screen(root); lv_obj_set_style_bg_opa(root,LV_OPA_COVER,0);
        auto layout=SceneLayout::create(root,44,48,false);
        BrandHeader::create(layout.top,100,tr(Text::Players));
        Container::setFlex(layout.body,LV_FLEX_FLOW_COLUMN,LV_FLEX_ALIGN_START,LV_FLEX_ALIGN_START);
        Label::create(layout.body,tr(Text::ChoosePlayers),&lv_font_montserrat_14,LV_TEXT_ALIGN_CENTER,LV_PCT(100));
        auto *row=Container::create(layout.body,LV_PCT(100),92);
        Container::setFlex(row,LV_FLEX_FLOW_ROW,LV_FLEX_ALIGN_CENTER,LV_FLEX_ALIGN_CENTER);
        count=Label::create(row,"1",UITheme::heading(40),LV_TEXT_ALIGN_CENTER,180);
        auto *arrows=Container::create(row,48,92);
        Container::setFlex(arrows); lv_obj_set_style_pad_row(arrows,4,0);
        up=Button::create(arrows,nullptr,change,this,44,44,LV_ALIGN_CENTER,0,0,LV_SYMBOL_UP);
        down=Button::create(arrows,nullptr,change,this,44,44,LV_ALIGN_CENTER,0,0,LV_SYMBOL_DOWN);
        UITheme::secondary(up); UITheme::secondary(down);
        Container::setFlex(layout.footer,LV_FLEX_FLOW_ROW,LV_FLEX_ALIGN_SPACE_EVENLY,LV_FLEX_ALIGN_CENTER);
        auto *cancel=Button::create(layout.footer,tr(Text::Cancel),cancelled,this,104,44);
        UITheme::secondary(cancel);
        Button::create(layout.footer,tr(Text::Start),confirmed,this,180,44,LV_ALIGN_CENTER,0,0,LV_SYMBOL_PLAY);
        hide();
    }
    void show(uint16_t initial=1) { players=initial ? initial:1; refresh(); lv_obj_remove_flag(root,LV_OBJ_FLAG_HIDDEN); lv_obj_move_foreground(root); }
    void hide() { if (root) lv_obj_add_flag(root,LV_OBJ_FLAG_HIDDEN); }
    bool visible() const { return root && !lv_obj_has_flag(root,LV_OBJ_FLAG_HIDDEN); }
    void clear() { root=count=up=down=nullptr; callback=nullptr; }
private:
    void refresh()
    {
        char text[8]; lv_snprintf(text,sizeof(text),"%u",unsigned(players)); Label::setText(count,text);
        lv_obj_set_state(down,LV_STATE_DISABLED,players==1); lv_obj_set_state(up,LV_STATE_DISABLED,players==UINT16_MAX);
    }
    static void change(lv_event_t *event)
    {
        auto *self=static_cast<PlayerSelectorDialog *>(lv_event_get_user_data(event));
        if (lv_event_get_target(event)==self->up && self->players<UINT16_MAX) ++self->players;
        else if (lv_event_get_target(event)==self->down && self->players>1) --self->players;
        self->refresh();
    }
    static void confirmed(lv_event_t *event)
    {
        auto *self=static_cast<PlayerSelectorDialog *>(lv_event_get_user_data(event));
        if (self->callback) self->callback(true,self->players);
    }
    static void cancelled(lv_event_t *event)
    {
        auto *self=static_cast<PlayerSelectorDialog *>(lv_event_get_user_data(event));
        if (self->callback) self->callback(false,self->players);
    }
    lv_obj_t *root=nullptr,*count=nullptr,*up=nullptr,*down=nullptr;
    uint16_t players=1;
    std::function<void(bool,uint16_t)> callback;
};
