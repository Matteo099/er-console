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
    constexpr lv_coord_t CARD_TITLE_WIDTH = 220;
    constexpr lv_coord_t CARD_LINE_WIDTH = 220;

    static void applyCardTheme(lv_obj_t* card)
    {
        if (!card)
            return;

        lv_obj_set_style_pad_all(card, 6, 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_20, 0);
        lv_obj_set_style_bg_color(card, lv_color_hex(0x111827), 0);
        lv_obj_set_style_border_width(card, 1, 0);
        lv_obj_set_style_border_color(card, lv_color_hex(0x1F2937), 0);
    }

    static lv_obj_t* createLine(lv_obj_t* card)
    {
        return Label::create(card, "-", &lv_font_roboto_mono_12, LV_TEXT_ALIGN_CENTER, CARD_LINE_WIDTH);
    }

    static lv_obj_t* createStatusCard(
        lv_obj_t* parent,
        const char* title,
        lv_obj_t*& line1,
        lv_obj_t*& line2,
        lv_obj_t*& line3)
    {
        lv_obj_t* card = Container::create(parent, CARD_WIDTH, LV_SIZE_CONTENT, LV_ALIGN_TOP_MID, 0, 0);
        Container::setFlex(card);
        applyCardTheme(card);

        lv_obj_t* heading = Label::create(card, title, &lv_font_roboto_mono_16, LV_TEXT_ALIGN_CENTER, CARD_TITLE_WIDTH);
        Label::setTextColor(heading, lv_color_hex(0x7DD3FC));

        line1 = createLine(card);
        line2 = createLine(card);
        line3 = createLine(card);
        return card;
    }

    static void setTripleLines(
        lv_obj_t* line1,
        lv_obj_t* line2,
        lv_obj_t* line3,
        const std::string& text1,
        const std::string& text2,
        const std::string& text3)
    {
        if (line1) lv_label_set_text(line1, text1.c_str());
        if (line2) lv_label_set_text(line2, text2.c_str());
        if (line3) lv_label_set_text(line3, text3.c_str());
    }
}

void MonitorView::create()
{
    screen = Screen::create();
    auto layout = SceneLayout::create(screen);
    brandImg = BrandHeader::create(layout.top);

    container = Container::create(layout.body, 240, LV_SIZE_CONTENT, LV_ALIGN_TOP_MID, 0, 0);
    Container::setFlex(container);

    powerCard = createStatusCard(container, "Power status", powerLine1, powerLine2, powerLine3);
    lv_obj_set_style_margin_top(powerCard, 10, 0);

    espCard = createStatusCard(container, "ESP32 status", espLine1, espLine2, espLine3);

    sdCard = createStatusCard(container, "SD status", sdLine1, sdLine2, sdLine3);
    lv_obj_set_style_margin_bottom(sdCard, 10, 0);

    btnBack = BackButton::create(layout.footer, onBackClicked, this);
}

void MonitorView::update()
{
}

void MonitorView::destroy()
{
    screen = nullptr;

    brandImg = nullptr;
    container = nullptr;

    powerCard = nullptr;
    powerLine1 = nullptr;
    powerLine2 = nullptr;
    powerLine3 = nullptr;

    espCard = nullptr;
    espLine1 = nullptr;
    espLine2 = nullptr;
    espLine3 = nullptr;

    sdCard = nullptr;
    sdLine1 = nullptr;
    sdLine2 = nullptr;
    sdLine3 = nullptr;

    btnBack = nullptr;
}

void MonitorView::setPowerStatus(const std::string& line1, const std::string& line2, const std::string& line3)
{
    setTripleLines(powerLine1, powerLine2, powerLine3, line1, line2, line3);
}

void MonitorView::setEspStatus(const std::string& line1, const std::string& line2, const std::string& line3)
{
    setTripleLines(espLine1, espLine2, espLine3, line1, line2, line3);
}

void MonitorView::setSdStatus(const std::string& line1, const std::string& line2, const std::string& line3)
{
    setTripleLines(sdLine1, sdLine2, sdLine3, line1, line2, line3);
}

void MonitorView::onBackClicked(lv_event_t* e)
{
    auto* view = static_cast<MonitorView*>(lv_event_get_user_data(e));
    if (!view)
        return;

    view->emitEvent(ButtonClickedEvent("back"));
}
