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
    constexpr lv_coord_t CARD_WIDTH = 220;

    static lv_obj_t *createCard(
        lv_obj_t *parent,
        const char *title)
    {
        auto *card =
            lv_obj_create(parent);

        lv_obj_set_size(
            card,
            CARD_WIDTH,
            LV_SIZE_CONTENT);

        lv_obj_set_style_bg_color(
            card,
            lv_color_hex(0x111827),
            0);

        lv_obj_set_style_bg_opa(
            card,
            LV_OPA_20,
            0);

        lv_obj_set_style_border_width(
            card,
            1,
            0);

        lv_obj_set_style_border_color(
            card,
            lv_color_hex(0x1F2937),
            0);

        lv_obj_set_style_pad_all(
            card,
            10,
            0);

        lv_obj_set_flex_flow(
            card,
            LV_FLEX_FLOW_COLUMN);

        lv_obj_set_style_pad_gap(
            card,
            8,
            0);

        auto *lbl =
            lv_label_create(card);

        lv_label_set_text(lbl, title);

        lv_obj_set_style_text_color(
            lbl,
            lv_color_hex(0x7DD3FC),
            0);

        return card;
    }

    static lv_obj_t *createMetricBar(
        lv_obj_t *parent,
        lv_obj_t *&valueLabel)
    {
        valueLabel =
            lv_label_create(parent);

        lv_label_set_text(
            valueLabel,
            "-");

        auto *bar =
            lv_bar_create(parent);

        lv_obj_set_width(
            bar,
            LV_PCT(100));

        lv_obj_set_height(
            bar,
            12);

        lv_bar_set_range(
            bar,
            0,
            100);

        return bar;
    }
}

void MonitorView::create()
{
    screen = Screen::create();

    auto layout = SceneLayout::create(screen);
    brandImg = BrandHeader::create(layout.top);
    container = Container::create(layout.body, 240, LV_SIZE_CONTENT, LV_ALIGN_TOP_MID, 0, 0);
    Container::setFlex(container);

    //
    // POWER CARD
    //
    powerCard = createCard(container, "Power");
    powerArc = lv_arc_create(powerCard);
    lv_obj_set_size(powerArc, 90, 90);
    lv_arc_set_range(powerArc, 0, 5000);
    // 0-5W
    powerValue = lv_label_create(powerArc);
    lv_label_set_text(powerValue, "0.0W");
    lv_obj_center(powerValue);
    voltageBar = lv_bar_create(powerCard);
    lv_bar_set_range(voltageBar, 0, 5500);
    // mV
    voltageLabel = lv_label_create(powerCard);
    currentBar = lv_bar_create(powerCard);
    lv_bar_set_range(currentBar, 0, 2000);
    // 2A
    currentLabel = lv_label_create(powerCard);

    //
    // ESP32 CARD
    //
    espCard = createCard(container, "ESP32");
    cpuArc = lv_arc_create(espCard);
    lv_obj_set_size(cpuArc, 90, 90);
    lv_arc_set_range(cpuArc, 0, 100);
    lv_arc_set_value(cpuArc, 0);
    cpuValue = lv_label_create(cpuArc);
    lv_label_set_text(cpuValue, "0%");
    lv_obj_center(cpuValue);
    cpuLabel = lv_label_create(cpuArc);
    lv_label_set_text(cpuLabel, "CPU");
    lv_obj_align(cpuLabel, LV_ALIGN_CENTER, 0, 18);
    uptimeLabel = lv_label_create(espCard);
    lv_label_set_text(uptimeLabel, "Uptime --");
    heapBar = createMetricBar(espCard, heapLabel);
    psramBar = createMetricBar(espCard, psramLabel);

    //
    // SD CARD
    //
    sdCard = createCard(container, "Storage");
    sdBar = lv_bar_create(sdCard);
    lv_obj_set_width(sdBar, LV_PCT(100));
    lv_obj_set_height(sdBar, 14);
    lv_bar_set_range(sdBar, 0, 100);
    sdUsedLabel = lv_label_create(sdCard);
    sdFreeLabel = lv_label_create(sdCard);

    btnBack = BackButton::create(layout.footer, onBackClicked, this);
}

void MonitorView::update()
{
}

void MonitorView::destroy()
{
    screen = nullptr;

    container = nullptr;
    brandImg = nullptr;
    btnBack = nullptr;
    powerCard = nullptr;
    espCard = nullptr;
    sdCard = nullptr;
    powerArc = nullptr;
    powerValue = nullptr;
    voltageBar = nullptr;
    voltageLabel = nullptr;
    currentBar = nullptr;
    currentLabel = nullptr;
    cpuArc = nullptr;
    cpuValue = nullptr;
    cpuLabel = nullptr;
    uptimeLabel = nullptr;
    heapBar = nullptr;
    heapLabel = nullptr;
    psramBar = nullptr;
    psramLabel = nullptr;
    sdBar = nullptr;
    sdUsedLabel = nullptr;
    sdFreeLabel = nullptr;
}

void MonitorView::setCpuUsage(
    uint8_t percent)
{
    lv_arc_set_value(cpuArc, percent);
    char buf[16];
    snprintf(buf, sizeof(buf), "%u%%", percent);
    lv_label_set_text(cpuValue, buf);
}

void MonitorView::setHeap(
    uint32_t usedKb,
    uint32_t totalKb)
{
    uint8_t pct = totalKb ? (usedKb * 100) / totalKb : 0;
    lv_bar_set_value(heapBar, pct, LV_ANIM_ON);
    char buf[64];
    snprintf(buf, sizeof(buf), "Heap %lu/%lu KB", usedKb, totalKb);
    lv_label_set_text(heapLabel, buf);
}

void MonitorView::setPsram(
    uint32_t usedKb,
    uint32_t totalKb)
{
    uint8_t pct = totalKb ? (usedKb * 100) / totalKb : 0;
    lv_bar_set_value(psramBar, pct, LV_ANIM_ON);
    char buf[64];
    snprintf(buf, sizeof(buf), "PSRAM %lu/%lu KB", usedKb, totalKb);
    lv_label_set_text(psramLabel, buf);
}

void MonitorView::setUptime(
    const char *text)
{
    lv_label_set_text(uptimeLabel, text);
}

void MonitorView::setSdStorage(
    uint32_t usedMb,
    uint32_t totalMb)
{
    uint32_t freeMb = totalMb - usedMb;
    uint8_t pct = totalMb ? (usedMb * 100) / totalMb : 0;
    lv_bar_set_value(sdBar, pct, LV_ANIM_ON);
    char buf[64];
    snprintf(buf, sizeof(buf), "Used %lu / %lu MB", usedMb, totalMb);
    lv_label_set_text(sdUsedLabel, buf);
    snprintf(buf, sizeof(buf), "Free %lu MB", freeMb);
    lv_label_set_text(sdFreeLabel, buf);
}

void MonitorView::setPower(
    float voltageV,
    float currentMa,
    float powerMw)
{
    lv_arc_set_value(powerArc, static_cast<int>(powerMw));
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2fW", powerMw / 1000.0f);
    lv_label_set_text(powerValue, buf);
    lv_bar_set_value(voltageBar, static_cast<int>(voltageV * 1000), LV_ANIM_ON);
    std::snprintf(buf, sizeof(buf), "%.2fV", voltageV);
    lv_label_set_text(voltageLabel, buf);
    lv_bar_set_value(currentBar, static_cast<int>(currentMa), LV_ANIM_ON);
    std::snprintf(buf, sizeof(buf), "%.0fmA", currentMa);
    lv_label_set_text(currentLabel, buf);
}

void MonitorView::onBackClicked(lv_event_t *e)
{
    auto *view = static_cast<MonitorView *>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("back"));
}
