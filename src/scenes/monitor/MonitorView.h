#pragma once
#include "systems/ui/View.h"

class MonitorView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;
    lv_obj_t *getScreen() override { return screen; }
    void setCpuUsage(uint8_t percent);
    void setRamUsage(uint32_t usedKb, uint32_t totalKb);
    void setHeap(uint32_t usedKb, uint32_t totalKb);
    void setPsram(uint32_t usedKb, uint32_t totalKb);
    void setUptime(const char *text);
    void setSdCapacity(uint32_t totalMb);
    void setPowerAvailable(bool available);
    void setPower(float voltageV, float currentMa, float powerMw);
private:
    static void onBackClicked(lv_event_t *e);
    lv_obj_t *screen = nullptr;
    lv_obj_t *cpuBar = nullptr, *cpuLabel = nullptr;
    lv_obj_t *ramBar = nullptr, *ramLabel = nullptr;
    lv_obj_t *heapLabel = nullptr, *psramLabel = nullptr, *uptimeLabel = nullptr;
    lv_obj_t *sdLabel = nullptr;
    lv_obj_t *powerCard = nullptr, *powerLabel = nullptr;
};
