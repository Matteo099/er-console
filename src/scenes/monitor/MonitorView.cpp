#include "MonitorView.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/BackButton.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"

namespace
{
    lv_obj_t *card(lv_obj_t *parent, const char *title)
    {
        auto *obj = Container::create(parent, LV_PCT(100), LV_SIZE_CONTENT);
        UITheme::card(obj);
        Container::setFlex(obj, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_set_style_pad_row(obj, 7, 0);
        auto *label = Label::create(obj, title, &lv_font_montserrat_12);
        UITheme::accent(label);
        return obj;
    }
    lv_obj_t *bar(lv_obj_t *parent)
    {
        auto *obj = lv_bar_create(parent);
        lv_obj_set_size(obj, LV_PCT(100), 7);
        lv_bar_set_range(obj, 0, 100);
        return obj;
    }
}
void MonitorView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen);
    BrandHeader::create(layout.top, 100, "Risorse e dispositivi");
    auto *column = Container::create(layout.body, LV_PCT(100), LV_SIZE_CONTENT, LV_ALIGN_TOP_MID);
    Container::setFlex(column);
    lv_obj_set_style_pad_row(column, 8, 0);
    auto *device = card(column, LV_SYMBOL_CHARGE " CONSOLE");
    cpuLabel = Label::create(device, "CPU", &lv_font_montserrat_14);
    cpuBar = bar(device);
    ramLabel = Label::create(device, "Memoria", &lv_font_montserrat_14);
    ramBar = bar(device);
    heapLabel = Label::create(device, "", &lv_font_montserrat_12);
    psramLabel = Label::create(device, "", &lv_font_montserrat_12);
    uptimeLabel = Label::create(device, "", &lv_font_montserrat_12);
    UITheme::muted(heapLabel);
    UITheme::muted(psramLabel);
    UITheme::muted(uptimeLabel);
    auto *sd = card(column, LV_SYMBOL_DRIVE " MICROSD");
    sdLabel = Label::create(sd, "", &lv_font_montserrat_14);
    powerCard = card(column, LV_SYMBOL_CHARGE " ALIMENTAZIONE");
    powerLabel = Label::create(powerCard, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    setPowerAvailable(false);
    BackButton::create(layout.footer, onBackClicked, this);
}
void MonitorView::update() {}
void MonitorView::destroy()
{
    screen = cpuBar = cpuLabel = ramBar = ramLabel = heapLabel = psramLabel = uptimeLabel = nullptr;
    sdLabel = powerCard = powerLabel = nullptr;
}
void MonitorView::setCpuUsage(uint8_t percent)
{
    char text[32];
    lv_snprintf(text, sizeof(text), "CPU  %u%%", percent);
    Label::setText(cpuLabel, text);
    lv_bar_set_value(cpuBar, percent, LV_ANIM_OFF);
}
void MonitorView::setRamUsage(uint32_t usedKb, uint32_t totalKb)
{
    const auto percent = totalKb ? (uint64_t(usedKb) * 100 / totalKb) : 0;
    char text[48];
    lv_snprintf(text, sizeof(text), "Memoria totale  %u%%", (unsigned)percent);
    Label::setText(ramLabel, text);
    lv_bar_set_value(ramBar, percent, LV_ANIM_OFF);
}
void MonitorView::setHeap(uint32_t usedKb, uint32_t totalKb)
{
    char text[64];
    lv_snprintf(text, sizeof(text), "RAM interna  %u / %u KB", (unsigned)usedKb, (unsigned)totalKb);
    Label::setText(heapLabel, text);
}
void MonitorView::setPsram(uint32_t usedKb, uint32_t totalKb)
{
    char text[64];
    lv_snprintf(text, sizeof(text), "PSRAM  %u / %u KB", (unsigned)usedKb, (unsigned)totalKb);
    Label::setText(psramLabel, text);
    lv_obj_set_flag(psramLabel, LV_OBJ_FLAG_HIDDEN, totalKb == 0);
}
void MonitorView::setUptime(const char *text) { Label::setText(uptimeLabel, text); }
void MonitorView::setSdCapacity(uint32_t totalMb)
{
    char text[48];
    if (totalMb) lv_snprintf(text, sizeof(text), "Pronta  /  %u.%u GB", (unsigned)(totalMb / 1024), (unsigned)((totalMb % 1024) * 10 / 1024));
    else lv_snprintf(text, sizeof(text), "SD non disponibile");
    Label::setText(sdLabel, text);
}
void MonitorView::setPowerAvailable(bool available) { lv_obj_set_flag(powerCard, LV_OBJ_FLAG_HIDDEN, !available); }
void MonitorView::setPower(float voltageV, float currentMa, float powerMw)
{
    char text[64];
    lv_snprintf(text, sizeof(text), "%.2f V   %.0f mA   %.2f W", voltageV, currentMa, powerMw / 1000.0f);
    Label::setText(powerLabel, text);
}
void MonitorView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<MonitorView *>(lv_event_get_user_data(e));
    if (view) view->emitEvent(ButtonClickedEvent("back"));
}
