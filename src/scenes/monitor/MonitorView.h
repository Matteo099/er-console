#pragma once

#include "systems/ui/View.h"
#include <string>
#include <lvgl.h>

class MonitorView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t *getScreen() override { return screen; }

    void setCpuUsage(uint8_t percent);

    void setHeap(
        uint32_t usedKb,
        uint32_t totalKb);

    void setPsram(
        uint32_t usedKb,
        uint32_t totalKb);

    void setUptime(const char *text);

    void setSdStorage(
        uint32_t usedMb,
        uint32_t totalMb);

    void setPower(
        float voltageV,
        float currentMa,
        float powerMw);

private:
    static void onBackClicked(lv_event_t *e);

    lv_obj_t *screen = nullptr;

    lv_obj_t *container = nullptr;
    lv_obj_t *brandImg = nullptr;
    lv_obj_t *btnBack = nullptr;

    lv_obj_t *powerCard = nullptr;
    lv_obj_t *espCard = nullptr;
    lv_obj_t *sdCard = nullptr;

    lv_obj_t *powerArc = nullptr;
    lv_obj_t *powerValue = nullptr;
    lv_obj_t *voltageBar = nullptr;
    lv_obj_t *voltageLabel = nullptr;
    lv_obj_t *currentBar = nullptr;
    lv_obj_t *currentLabel = nullptr;

    lv_obj_t *cpuArc = nullptr;
    lv_obj_t *cpuValue = nullptr;
    lv_obj_t *cpuLabel = nullptr;

    lv_obj_t *uptimeLabel = nullptr;

    lv_obj_t *heapBar = nullptr;
    lv_obj_t *heapLabel = nullptr;

    lv_obj_t *psramBar = nullptr;
    lv_obj_t *psramLabel = nullptr;

    lv_obj_t *sdBar = nullptr;
    lv_obj_t *sdUsedLabel = nullptr;
    lv_obj_t *sdFreeLabel = nullptr;
};