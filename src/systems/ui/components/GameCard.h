#pragma once

#include <lvgl.h>
#include <string>

class GameCard
{
public:
    struct GameData
    {
        std::string title;
        std::string description;
        std::string author;
        int difficulty;
        int durationMin;
    };

private:
    static lv_obj_t *createRow(lv_obj_t *parent)
    {
        lv_obj_t *row = lv_obj_create(parent);
        if (!row)
            return nullptr;

        lv_obj_remove_style_all(row);

        lv_obj_set_width(row, LV_PCT(100));
        lv_obj_set_height(row, LV_SIZE_CONTENT);

        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(
            row,
            LV_FLEX_ALIGN_START,
            LV_FLEX_ALIGN_CENTER,
            LV_FLEX_ALIGN_CENTER);

        lv_obj_set_style_pad_gap(row, 6, 0);
        lv_obj_set_style_pad_top(row, 4, 0);
        lv_obj_set_style_pad_bottom(row, 4, 0);

        return row;
    }

    static lv_obj_t *icon(lv_obj_t *parent, const char *symbol)
    {
        lv_obj_t *l = lv_label_create(parent);
        if (!l)
            return nullptr;

        lv_label_set_text(l, symbol ? symbol : "");
        return l;
    }

    static lv_obj_t *text(lv_obj_t *parent, const char *str, const lv_font_t *font)
    {
        lv_obj_t *l = lv_label_create(parent);
        if (!l)
            return nullptr;

        lv_label_set_text(l, str ? str : "");
        lv_obj_set_style_text_font(l, font, 0);

        return l;
    }

    static lv_obj_t *divider(lv_obj_t *parent)
    {
        lv_obj_t *line = lv_obj_create(parent);
        if (!line)
            return nullptr;

        lv_obj_remove_style_all(line);

        lv_obj_set_width(line, LV_PCT(100));
        lv_obj_set_height(line, 1);

        lv_obj_set_style_bg_color(line, lv_color_hex(0x444444), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);

        return line;
    }

public:
    static lv_obj_t *create(lv_obj_t *parent, const GameData &data)
    {
        if (!parent)
            return nullptr;

        lv_obj_t *card = lv_obj_create(parent);
        if (!card)
            return nullptr;

        lv_obj_remove_style_all(card);

        lv_obj_set_width(card, LV_PCT(100));
        lv_obj_set_height(card, LV_SIZE_CONTENT);

        lv_obj_set_style_radius(card, 12, 0);
        lv_obj_set_style_pad_all(card, 12, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(card, 0, 0);

        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(
            card,
            LV_FLEX_ALIGN_START,
            LV_FLEX_ALIGN_START, // TODO: revert - LV_FLEX_ALIGN_CENTER
            LV_FLEX_ALIGN_CENTER);

        // Title
        lv_obj_t *title = lv_label_create(card);
        lv_label_set_text(title, data.title.c_str());
        lv_obj_set_style_text_font(title, &lv_font_roboto_mono_20, 0);
        // TODO: revert - lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_width(title, LV_PCT(100));

        lv_obj_set_style_pad_bottom(title, 6, 0);

        // Description
        lv_obj_t *desc = lv_label_create(card);
        lv_label_set_text(desc, data.description.c_str());
        lv_obj_set_style_text_font(desc, &lv_font_roboto_mono_12, 0);
        lv_obj_set_width(desc, LV_PCT(100));
        // TODO: revert - lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_bottom(desc, 8, 0);

        divider(card);

        char buf[64];

        // Difficulty
        lv_obj_t *diffRow = createRow(card);
        icon(diffRow, LV_SYMBOL_SETTINGS);
        snprintf(buf, sizeof(buf), "Difficulty: %d/5", data.difficulty);
        text(diffRow, buf, &lv_font_roboto_mono_14);

        // Duration
        lv_obj_t *durationRow = createRow(card);
        icon(durationRow, LV_SYMBOL_REFRESH);
        snprintf(buf, sizeof(buf), "%d minutes", data.durationMin);
        text(durationRow, buf, &lv_font_roboto_mono_14);

        // Author
        lv_obj_t *authorRow = createRow(card);
        icon(authorRow, LV_SYMBOL_DIRECTORY);
        text(authorRow, data.author.c_str(), &lv_font_roboto_mono_14);

        return card;
    }
};