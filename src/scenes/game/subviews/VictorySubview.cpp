#include "VictorySubview.h"

#include "systems/ui/components/Button.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/Align.h"

#include <cstdio>

void VictorySubview::create(lv_obj_t *parent)
{
    root = Container::create(parent, LV_PCT(100), LV_PCT(100));
    titleLabel = Label::create(root, "YOU WIN!", &lv_font_roboto_mono_28, LV_TEXT_ALIGN_CENTER, 240);
    Label::setTextColor(titleLabel, lv_color_hex(0x00FF00));
    Align::setPosition(titleLabel, LV_ALIGN_TOP_MID, 0, 25);
    statsContainer = Container::create(root, 240, 150, LV_ALIGN_CENTER, 0, -10);
    lv_obj_set_flex_flow(statsContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(statsContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    elapsedRow = StatisticRow::create(statsContainer, "Elapsed");
    pausedRow = StatisticRow::create(statsContainer, "Paused");
    hintsRow = StatisticRow::create(statsContainer, "Hints");
    solvedRow = StatisticRow::create(statsContainer, "Solved");
    completeBtn = Button::create(root, "Complete", onComplete, this, 150, 44, LV_ALIGN_BOTTOM_MID, 0, -20, LV_SYMBOL_OK);
    hide();
}

void VictorySubview::destroy()
{
    root = nullptr;
    titleLabel = nullptr;
    statsContainer = nullptr;
    completeBtn = nullptr;
}

void VictorySubview::show()
{
    if (root)
        lv_obj_clear_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void VictorySubview::hide()
{
    if (root)
        lv_obj_add_flag(root, LV_OBJ_FLAG_HIDDEN);
}

void VictorySubview::setEventCallback(EventCallback callback) { eventCallback = std::move(callback); }

void VictorySubview::setStats(const GameStats &stats)
{
    StatisticRow::setValue(elapsedRow, formatTime(stats.elapsedSeconds).c_str());
    StatisticRow::setValue(pausedRow, formatTime(stats.pausedSeconds).c_str());
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)stats.hintsUsed);
    StatisticRow::setValue(hintsRow, buffer);
    snprintf(buffer, sizeof(buffer), "%lu", (unsigned long)stats.puzzlesSolved);
    StatisticRow::setValue(solvedRow, buffer);
}

void VictorySubview::onComplete(lv_event_t *e)
{
    auto *view = static_cast<VictorySubview *>(lv_event_get_user_data(e));
    if (!view)
        return;
    if (view->eventCallback)
        view->eventCallback("complete");
}

std::string VictorySubview::formatTime(uint32_t seconds)
{
    char buffer[16];
    snprintf(buffer,
             sizeof(buffer),
             "%02lu:%02lu:%02lu",
             (unsigned long)(seconds / 3600),
             (unsigned long)((seconds % 3600) / 60),
             (unsigned long)(seconds % 60));
    return buffer;
}