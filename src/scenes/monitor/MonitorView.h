#pragma once

#include "systems/ui/View.h"
#include "systems/ui/UICleanup.h"
#include <string>

class MonitorView : public View
{
public:
    void create() override;
    void update() override;
    void destroy() override;

    lv_obj_t* getScreen() override { return screen; }

    void setPowerStatus(const std::string& line1, const std::string& line2, const std::string& line3);
    void setEspStatus(const std::string& line1, const std::string& line2, const std::string& line3);
    void setSdStatus(const std::string& line1, const std::string& line2, const std::string& line3);

private:
    static void onBackClicked(lv_event_t* e);

private:
    lv_obj_t* screen = nullptr;
    lv_obj_t* brandImg = nullptr;
    lv_obj_t* container = nullptr;

    lv_obj_t* powerCard = nullptr;
    lv_obj_t* powerLine1 = nullptr;
    lv_obj_t* powerLine2 = nullptr;
    lv_obj_t* powerLine3 = nullptr;

    lv_obj_t* espCard = nullptr;
    lv_obj_t* espLine1 = nullptr;
    lv_obj_t* espLine2 = nullptr;
    lv_obj_t* espLine3 = nullptr;

    lv_obj_t* sdCard = nullptr;
    lv_obj_t* sdLine1 = nullptr;
    lv_obj_t* sdLine2 = nullptr;
    lv_obj_t* sdLine3 = nullptr;

    lv_obj_t* btnBack = nullptr;
};
