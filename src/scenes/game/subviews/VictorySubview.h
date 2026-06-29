#pragma once

#include <lvgl.h>
#include <functional>
#include <cstdint>

#include "systems/ui/components/StatisticRow.h"

class VictorySubview
{
public:
    struct GameStats
    {
        uint32_t elapsedSeconds = 0;
        uint32_t pausedSeconds = 0;
        uint32_t hintsUsed = 0;
        uint32_t puzzlesSolved = 0;
    };

    using EventCallback =
        std::function<void(const char *)>;

    void create(lv_obj_t *parent);
    void destroy();

    void show();
    void hide();

    void setEventCallback(
        EventCallback callback);

    void setStats(
        const GameStats &stats);

private:
    static void onComplete(
        lv_event_t *e);

    static std::string formatTime(
        uint32_t seconds);

private:
    EventCallback eventCallback;

    lv_obj_t *root = nullptr;

    lv_obj_t *titleLabel = nullptr;

    lv_obj_t *statsContainer = nullptr;

    lv_obj_t *completeBtn = nullptr;

    StatisticRow::Parts elapsedRow;
    StatisticRow::Parts pausedRow;
    StatisticRow::Parts hintsRow;
    StatisticRow::Parts solvedRow;
};