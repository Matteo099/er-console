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

    static lv_obj_t* create(
        lv_obj_t* parent,
        const GameData& gameData,
        lv_event_cb_t callback,
        void* userData,
        lv_coord_t width = 200,
        lv_coord_t x = 0,
        lv_coord_t y = 0)
    {
        lv_obj_t* card = lv_btn_create(parent);
        lv_obj_set_width(card, width);
        lv_obj_set_style_radius(card, 8, 0);
        lv_obj_set_style_pad_all(card, 10, 0);
        lv_obj_add_event_cb(card, callback, LV_EVENT_CLICKED, userData);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

        // Title with play icon
        lv_obj_t* titleContainer = lv_obj_create(card);
        lv_obj_set_size(titleContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(titleContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(titleContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_opa(titleContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(titleContainer, 0, 0);
        lv_obj_set_style_pad_gap(titleContainer, 4, 0);

        lv_obj_t* playIcon = lv_label_create(titleContainer);
        lv_label_set_text(playIcon, LV_SYMBOL_PLAY);
        lv_obj_set_style_text_font(playIcon, &lv_font_montserrat_14, 0);

        lv_obj_t* title = lv_label_create(titleContainer);
        lv_label_set_text(title, gameData.title.c_str());
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);

        // Difficulty indicator (collapsed view)
        lv_obj_t* diffContainer = lv_obj_create(card);
        lv_obj_set_size(diffContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(diffContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_bg_opa(diffContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(diffContainer, 0, 0);
        lv_obj_set_style_pad_gap(diffContainer, 4, 0);

        lv_obj_t* diffIcon = lv_label_create(diffContainer);
        lv_label_set_text(diffIcon, LV_SYMBOL_SETTINGS);
        lv_obj_set_style_text_font(diffIcon, &lv_font_montserrat_12, 0);

        lv_obj_t* diffLabel = lv_label_create(diffContainer);
        lv_label_set_text_fmt(diffLabel, "Difficulty %d/5", gameData.difficulty);
        lv_obj_set_style_text_font(diffLabel, &lv_font_montserrat_12, 0);

        return card;
    }

    static void setExpanded(
        lv_obj_t* card,
        const GameData& gameData,
        lv_event_cb_t startButtonCallback,
        void* userData)
    {
        if (!card)
            return;

        // Clean existing content
        lv_obj_clean(card);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_gap(card, 6, 0);

        // Title with play icon
        lv_obj_t* titleContainer = lv_obj_create(card);
        lv_obj_set_size(titleContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(titleContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(titleContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_opa(titleContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(titleContainer, 0, 0);
        lv_obj_set_style_pad_gap(titleContainer, 4, 0);

        lv_obj_t* playIcon = lv_label_create(titleContainer);
        lv_label_set_text(playIcon, LV_SYMBOL_PLAY);
        lv_obj_set_style_text_font(playIcon, &lv_font_montserrat_16, 0);

        lv_obj_t* title = lv_label_create(titleContainer);
        lv_label_set_text(title, gameData.title.c_str());
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);

        // Description
        lv_obj_t* desc = lv_label_create(card);
        lv_label_set_text(desc, gameData.description.c_str());
        lv_obj_set_style_text_font(desc, &lv_font_montserrat_12, 0);
        lv_obj_set_width(desc, 180);

        // Difficulty with indicator
        lv_obj_t* diffContainer = lv_obj_create(card);
        lv_obj_set_size(diffContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(diffContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_bg_opa(diffContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(diffContainer, 0, 0);
        lv_obj_set_style_pad_gap(diffContainer, 4, 0);

        lv_obj_t* diffIcon = lv_label_create(diffContainer);
        lv_label_set_text(diffIcon, LV_SYMBOL_SETTINGS);
        lv_obj_set_style_text_font(diffIcon, &lv_font_montserrat_12, 0);

        lv_obj_t* diffLabel = lv_label_create(diffContainer);
        lv_label_set_text_fmt(diffLabel, "Level %d/5", gameData.difficulty);
        lv_obj_set_style_text_font(diffLabel, &lv_font_montserrat_12, 0);

        // Duration with clock icon
        lv_obj_t* durationContainer = lv_obj_create(card);
        lv_obj_set_size(durationContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(durationContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_bg_opa(durationContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(durationContainer, 0, 0);
        lv_obj_set_style_pad_gap(durationContainer, 4, 0);

        lv_obj_t* clockIcon = lv_label_create(durationContainer);
        lv_label_set_text(clockIcon, LV_SYMBOL_REFRESH);
        lv_obj_set_style_text_font(clockIcon, &lv_font_montserrat_12, 0);

        lv_obj_t* durationLabel = lv_label_create(durationContainer);
        lv_label_set_text_fmt(durationLabel, "%d minutes", gameData.durationMin);
        lv_obj_set_style_text_font(durationLabel, &lv_font_montserrat_12, 0);

        // Start button
        lv_obj_t* btnContainer = lv_obj_create(card);
        lv_obj_set_size(btnContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(btnContainer, LV_FLEX_FLOW_ROW_REVERSE);
        lv_obj_set_style_bg_opa(btnContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(btnContainer, 0, 0);

        lv_obj_t* btnStart = lv_btn_create(btnContainer);
        lv_obj_set_size(btnStart, 80, 32);
        lv_obj_set_style_radius(btnStart, 6, 0);
        lv_obj_add_event_cb(btnStart, startButtonCallback, LV_EVENT_CLICKED, userData);
        lv_obj_set_flex_flow(btnStart, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(btnStart, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_gap(btnStart, 4, 0);

        lv_obj_t* playBtnIcon = lv_label_create(btnStart);
        lv_label_set_text(playBtnIcon, LV_SYMBOL_PLAY);
        lv_obj_set_style_text_font(playBtnIcon, &lv_font_montserrat_12, 0);

        lv_obj_t* btnLabel = lv_label_create(btnStart);
        lv_label_set_text(btnLabel, "Start");
        lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_12, 0);
    }

    static void setCollapsed(
        lv_obj_t* card,
        const GameData& gameData)
    {
        if (!card)
            return;

        // Clean existing content
        lv_obj_clean(card);
        lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_gap(card, 4, 0);

        // Title with play icon
        lv_obj_t* titleContainer = lv_obj_create(card);
        lv_obj_set_size(titleContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(titleContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(titleContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_bg_opa(titleContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(titleContainer, 0, 0);
        lv_obj_set_style_pad_gap(titleContainer, 4, 0);

        lv_obj_t* playIcon = lv_label_create(titleContainer);
        lv_label_set_text(playIcon, LV_SYMBOL_PLAY);
        lv_obj_set_style_text_font(playIcon, &lv_font_montserrat_14, 0);

        lv_obj_t* title = lv_label_create(titleContainer);
        lv_label_set_text(title, gameData.title.c_str());
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);

        // Difficulty indicator
        lv_obj_t* diffContainer = lv_obj_create(card);
        lv_obj_set_size(diffContainer, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_flex_flow(diffContainer, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_bg_opa(diffContainer, LV_OPA_TRANSP, 0);
        lv_obj_set_style_pad_all(diffContainer, 0, 0);
        lv_obj_set_style_pad_gap(diffContainer, 4, 0);

        lv_obj_t* diffIcon = lv_label_create(diffContainer);
        lv_label_set_text(diffIcon, LV_SYMBOL_SETTINGS);
        lv_obj_set_style_text_font(diffIcon, &lv_font_montserrat_12, 0);

        lv_obj_t* diffLabel = lv_label_create(diffContainer);
        lv_label_set_text_fmt(diffLabel, "Difficulty %d/5", gameData.difficulty);
        lv_obj_set_style_text_font(diffLabel, &lv_font_montserrat_12, 0);
    }
};
