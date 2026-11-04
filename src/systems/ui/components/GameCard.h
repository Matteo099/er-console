#pragma once
#include <lvgl.h>
#include <string>
#include "Container.h"
#include "Label.h"

class GameCard
{
public:
    struct GameData
    {
        std::string title, description, author;
        int difficulty, durationMin;
    };
    static lv_obj_t *create(lv_obj_t *parent, const GameData &data)
    {
        auto *card = Container::create(parent, LV_PCT(100), LV_SIZE_CONTENT, LV_ALIGN_TOP_MID);
        UITheme::card(card);
        Container::setFlex(card, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(card, 9, 0);
        Label::create(card, data.title.c_str(), UITheme::heading(26), LV_TEXT_ALIGN_LEFT, LV_PCT(100));
        if (!data.description.empty())
        {
            auto *description = Label::create(card, data.description.c_str(), &lv_font_montserrat_12,
                                               LV_TEXT_ALIGN_LEFT, LV_PCT(100));
            UITheme::muted(description);
        }
        char detail[80];
        const int difficulty = data.difficulty < 1 ? 1 : data.difficulty > 5 ? 5 : data.difficulty;
        if (data.durationMin > 0)
            lv_snprintf(detail, sizeof(detail), LV_SYMBOL_REFRESH " %d min     Livello %d/5", data.durationMin, difficulty);
        else
            lv_snprintf(detail, sizeof(detail), "Durata libera     Livello %d/5", difficulty);
        auto *meta = Label::create(card, detail, &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
        UITheme::accent(meta);
        if (!data.author.empty())
        {
            auto *author = Label::create(card, data.author.c_str(), &lv_font_montserrat_12,
                                         LV_TEXT_ALIGN_LEFT, LV_PCT(100));
            UITheme::muted(author);
        }
        return card;
    }
};
