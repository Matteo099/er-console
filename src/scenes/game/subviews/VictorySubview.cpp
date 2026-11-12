#include "systems/ui/localization/I18n.h"
#include "VictorySubview.h"

#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/Align.h"
#include "systems/ui/components/SceneLayout.h"
#include "log/Logger.h"

#include <cstdio>

namespace {
constexpr const char* TAG = "VictorySubview";
}

void VictorySubview::create(lv_obj_t* parent) {
    Log::info(TAG, "Creating victory subview");

    root = Container::create(parent, LV_PCT(100), LV_PCT(100));

    // Use SceneLayout instead of manual positioning
    auto layout = SceneLayout::create(root);

    lv_obj_t* header = layout.top;
    lv_obj_t* body = layout.body;
    lv_obj_t* footer = layout.footer;

    // ----------------------------
    // HEADER (Title)
    // ----------------------------
    titleLabel = Label::create(header, tr(Text::Victory), UITheme::heading(26),
                               LV_TEXT_ALIGN_CENTER, LV_PCT(100));

    UITheme::accent(titleLabel);
    Align::setPosition(titleLabel, LV_ALIGN_CENTER, 0, 0);

    // ----------------------------
    // BODY (Stats)
    // ----------------------------
    statsContainer = Container::create(body, LV_PCT(100), LV_PCT(100));

    lv_obj_set_flex_flow(statsContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(statsContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    lv_obj_set_style_pad_all(statsContainer, 5, 0);
    // lv_obj_set_style_pad_row(statsContainer, 5, 0);

    lv_obj_add_flag(statsContainer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(statsContainer, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(statsContainer, LV_SCROLLBAR_MODE_AUTO);

    elapsedRow = StatisticRow::create(statsContainer, tr(Text::TimePlayed));
    pausedRow = StatisticRow::create(statsContainer, tr(Text::Pause));
    hintsRow = StatisticRow::create(statsContainer, tr(Text::Hints));
    solvedRow = StatisticRow::create(statsContainer, tr(Text::Objectives));
    playersRow = StatisticRow::create(statsContainer, tr(Text::Players));

    // ----------------------------
    // FOOTER (Button)
    // ----------------------------
    completeBtn = Button::create(footer, tr(Text::Continue), onComplete, this, 150, 44,
                                 LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_OK);

    hide();

    Log::info(TAG, "Victory subview created");
}

void VictorySubview::destroy() {
    Log::info(TAG, "Destroying victory subview");

    root = nullptr;
    titleLabel = nullptr;
    statsContainer = nullptr;
    completeBtn = nullptr;

    Log::info(TAG, "Victory subview destroyed");
}

void VictorySubview::show() {
    Log::debug(TAG, "Showing victory subview");
    if (!root || !lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN))
        return;
    lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
    playAudioFile(AUDIO_VICTORY_FILE, false, AudioChannel::Effects);
}

void VictorySubview::hide() {
    Log::debug(TAG, "Hiding victory subview");

    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void VictorySubview::setEventCallback(EventCallback callback) {
    Log::debug(TAG, "Event callback registered");
    eventCallback = std::move(callback);
}

void VictorySubview::setStats(const GameStats& stats) {
    char players[8];
    lv_snprintf(players, sizeof(players), "%u", unsigned(stats.players));
    StatisticRow::setValue(playersRow, players);
    Log::info(TAG, "Updating stats: elapsed=%lu paused=%lu hints=%lu solved=%lu",
              static_cast<unsigned long>(stats.elapsedSeconds),
              static_cast<unsigned long>(stats.pausedSeconds),
              static_cast<unsigned long>(stats.hintsUsed),
              static_cast<unsigned long>(stats.puzzlesSolved));

    StatisticRow::setValue(elapsedRow, formatTime(stats.elapsedSeconds).c_str());

    StatisticRow::setValue(pausedRow, formatTime(stats.pausedSeconds).c_str());

    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%lu", static_cast<unsigned long>(stats.hintsUsed));

    StatisticRow::setValue(hintsRow, buffer);

    snprintf(buffer, sizeof(buffer), "%lu", static_cast<unsigned long>(stats.puzzlesSolved));

    StatisticRow::setValue(solvedRow, buffer);
}

void VictorySubview::onComplete(lv_event_t* e) {
    auto* view = static_cast<VictorySubview*>(lv_event_get_user_data(e));

    if (!view) {
        Log::warn(TAG, "Complete callback received null view");
        return;
    }

    Log::debug(TAG, "Complete button clicked");

    if (view->eventCallback)
        view->eventCallback("complete");
}

std::string VictorySubview::formatTime(uint32_t seconds) {
    char buffer[16];

    snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu",
             static_cast<unsigned long>(seconds / 3600),
             static_cast<unsigned long>((seconds % 3600) / 60),
             static_cast<unsigned long>(seconds % 60));

    return buffer;
}
