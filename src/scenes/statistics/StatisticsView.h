#pragma once
#include "systems/ui/View.h"
#include "systems/ui/components/StatisticRow.h"
#include "services/StatisticsRepository.h"

class StatisticsView : public View
{
public:
    void create() override;
    void update() override {}
    void destroy() override { screen=title=subtitle=hint=status=nullptr; }
    lv_obj_t *getScreen() override { return screen; }
    void show(const StatisticsSnapshot &value,const std::string &gameTitle);
private:
    static void onBack(lv_event_t *event);
    lv_obj_t *screen=nullptr,*title=nullptr,*subtitle=nullptr,*hint=nullptr,*status=nullptr;
    StatisticRow::Parts rows[10]{};
};
