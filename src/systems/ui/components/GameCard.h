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
        int difficulty;
        int durationMin;
    };

private:
    static lv_obj_t *createContainer(
        lv_obj_t *parent,
        lv_flex_flow_t flow = LV_FLEX_FLOW_ROW)
    {
        lv_obj_t *obj = lv_obj_create(parent);

        lv_obj_remove_style_all(obj);

        lv_obj_set_width(obj, LV_PCT(100));
        lv_obj_set_height(obj, LV_SIZE_CONTENT);

        lv_obj_set_flex_flow(obj, flow);
        lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_set_style_pad_gap(obj, 4, 0);

        return obj;
    }

    static lv_obj_t *createIcon(
        lv_obj_t *parent,
        const char *symbol,
        const lv_font_t *font)
    {
        lv_obj_t *label = lv_label_create(parent);
        lv_label_set_text(label, symbol);
        lv_obj_set_style_text_font(label, font, 0);
        return label;
    }

    static lv_obj_t *createLabel(
        lv_obj_t *parent,
        const char *text,
        const lv_font_t *font)
    {
        lv_obj_t *label = lv_label_create(parent);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_font(label, font, 0);
        return label;
    }

    static lv_obj_t *createTitleRow(
        lv_obj_t *parent,
        const std::string &titleText,
        const lv_font_t *iconFont = &lv_font_montserrat_14)
    {
        auto *row = createContainer(parent);

        createIcon(row, LV_SYMBOL_PLAY, iconFont);
        createLabel(row, titleText.c_str(), &lv_font_montserrat_16);

        return row;
    }

    static lv_obj_t *createInfoRow(
        lv_obj_t *parent,
        const char *icon,
        const char *text)
    {
        auto *row = createContainer(parent);

        createIcon(row, icon, &lv_font_montserrat_12);
        createLabel(row, text, &lv_font_montserrat_12);

        return row;
    }

    static void configureCard(lv_obj_t *card, uint8_t gap)
    {
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_gap(card, gap, 0);
    }

public:
    static lv_obj_t *create(
        lv_obj_t *parent,
        const GameData &gameData,
        lv_event_cb_t callback,
        void *userData,
        lv_coord_t width = 200,
        lv_coord_t = 0,
        lv_coord_t = 0)
    {
        lv_obj_t *card = lv_btn_create(parent);

        lv_obj_set_width(card, width);
        lv_obj_set_style_radius(card, 8, 0);
        lv_obj_set_style_pad_all(card, 10, 0);
        lv_obj_add_event_cb(card, callback, LV_EVENT_CLICKED, userData);

        configureCard(card, 4);
        createTitleRow(card, gameData.title);
        char diff[32];
        snprintf(diff, sizeof(diff), "Difficulty %d/5", gameData.difficulty);
        createInfoRow(card, LV_SYMBOL_SETTINGS, diff);

        return card;
    }

    static void setCollapsed(
        lv_obj_t *card,
        const GameData &gameData)
    {
        if (!card)
            return;

        lv_obj_clean(card);

        configureCard(card, 4);
        createTitleRow(card, gameData.title);
        char diff[32];
        snprintf(diff, sizeof(diff), "Difficulty %d/5", gameData.difficulty);
        createInfoRow(card, LV_SYMBOL_SETTINGS, diff);
    }

    static void setExpanded(
        lv_obj_t *card,
        const GameData &gameData,
        lv_event_cb_t startButtonCallback,
        void *userData)
    {
        if (!card)
            return;

        lv_obj_clean(card);
        configureCard(card, 6);
        createTitleRow(card, gameData.title, &lv_font_montserrat_16);

        lv_obj_t *desc = lv_label_create(card);
        lv_label_set_text(desc, gameData.description.c_str());
        lv_obj_set_style_text_font(desc, &lv_font_montserrat_12, 0);
        lv_obj_set_width(desc, 180);

        char diff[32];
        snprintf(diff, sizeof(diff), "Level %d/5", gameData.difficulty);
        createInfoRow(card, LV_SYMBOL_SETTINGS, diff);

        char duration[32];
        snprintf(duration, sizeof(duration), "%d minutes", gameData.durationMin);
        createInfoRow(card, LV_SYMBOL_REFRESH, duration);

        auto *btnContainer = createContainer(card, LV_FLEX_FLOW_ROW_REVERSE);
        lv_obj_t *btnStart = lv_btn_create(btnContainer);
        lv_obj_set_size(btnStart, 80, 32);
        lv_obj_set_style_radius(btnStart, 6, 0);
        lv_obj_add_event_cb(btnStart, startButtonCallback, LV_EVENT_CLICKED, userData);
        lv_obj_set_flex_flow(btnStart, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btnStart, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        createIcon(btnStart, LV_SYMBOL_PLAY, &lv_font_montserrat_12);
        createLabel(btnStart, "Start", &lv_font_montserrat_12);
    }
};