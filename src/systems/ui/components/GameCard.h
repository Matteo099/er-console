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
        lv_obj_set_style_pad_all(card, 8, 0);
        lv_obj_add_event_cb(card, callback, LV_EVENT_CLICKED, userData);

        lv_obj_t* title = lv_label_create(card);
        lv_label_set_text(title, gameData.title.c_str());
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
        lv_obj_set_width(title, width - 16);
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

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

        // Recreate title
        lv_obj_t* title = lv_label_create(card);
        lv_label_set_text(title, gameData.title.c_str());
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
        lv_obj_set_width(title, 200 - 16);
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);

        // Add description and details
        lv_obj_t* desc = lv_label_create(card);
        lv_label_set_text_fmt(
            desc,
            "%s\n\nDifficulty: %d/5\nDuration: %d min",
            gameData.description.c_str(),
            gameData.difficulty,
            gameData.durationMin);
        lv_obj_set_style_text_font(desc, &lv_font_montserrat_14, 0);
        lv_obj_set_width(desc, 184);
        lv_obj_align(desc, LV_ALIGN_TOP_LEFT, 0, 28);

        // Add start button
        lv_obj_t* btnStart = lv_btn_create(card);
        lv_obj_set_size(btnStart, 80, 32);
        lv_obj_align(btnStart, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_add_event_cb(btnStart, startButtonCallback, LV_EVENT_CLICKED, userData);

        lv_obj_t* btnLabel = lv_label_create(btnStart);
        lv_label_set_text(btnLabel, "Start");
        lv_obj_center(btnLabel);
    }

    static void setCollapsed(
        lv_obj_t* card,
        const GameData& gameData)
    {
        if (!card)
            return;

        // Clean existing content
        lv_obj_clean(card);

        // Recreate only the title
        lv_obj_t* title = lv_label_create(card);
        lv_label_set_text(title, gameData.title.c_str());
        lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
        lv_obj_set_width(title, 200 - 16);
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 0, 0);
    }
};
