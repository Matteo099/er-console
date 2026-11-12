#include "StatisticsView.h"
#include "systems/ui/UIEvents.h"
#include "systems/ui/components/Screen.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Container.h"
#include "systems/ui/components/Button.h"

namespace {
const Text keys[] = {Text::GamesPlayed,    Text::Wins,
                     Text::Losses,         Text::AbandonedGames,
                     Text::ActiveGames,    Text::AverageDuration,
                     Text::BestCompletion, Text::WorstCompletion,
                     Text::TotalPlayers,   Text::LastPlayers};

std::string number(uint64_t value) {
    char text[24];
    lv_snprintf(text, sizeof(text), "%llu", static_cast<unsigned long long>(value));
    return text;
}

std::string duration(uint64_t milliseconds) {
    const auto seconds = milliseconds / 1000;
    char text[40];
    lv_snprintf(text, sizeof(text), "%02llu:%02llu:%02llu",
                static_cast<unsigned long long>(seconds / 3600),
                static_cast<unsigned long long>((seconds / 60) % 60),
                static_cast<unsigned long long>(seconds % 60));
    return text;
}
}  // namespace

void StatisticsView::create() {
    screen = Screen::create();
    auto layout = SceneLayout::create(screen);
    auto* header = BrandHeader::create(layout.top, 100, tr(Text::Statistics));
    title = lv_obj_get_child(header, 1);
    Container::setFlex(layout.body, LV_FLEX_FLOW_COLUMN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(layout.body, 4, 0);
    subtitle =
        Label::create(layout.body, "", UITheme::heading(18), LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    hint = Label::create(layout.body, tr(Text::StatisticsHint), &lv_font_montserrat_12,
                         LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(hint);
    for (unsigned i = 0; i < 10; ++i) {
        rows[i] = StatisticRow::create(layout.body, tr(keys[i]));
        lv_obj_set_width(rows[i].value, 104);
        lv_obj_set_flex_grow(rows[i].title, 1);
        lv_label_set_long_mode(rows[i].title, LV_LABEL_LONG_DOT);
        UITheme::card(rows[i].container);
        lv_obj_set_style_pad_all(rows[i].container, 6, 0);
        lv_obj_set_height(rows[i].container, 28);
        lv_obj_set_style_pad_ver(rows[i].container, 4, 0);
        Label::setFont(rows[i].title, &lv_font_montserrat_12);
        Label::setFont(rows[i].value, &lv_font_roboto_mono_14);
        if (i == 8) {
            auto* note = Label::create(layout.body, tr(Text::ParticipantHint),
                                       &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
            UITheme::muted(note);
        }
    }
    status =
        Label::create(layout.body, "", &lv_font_montserrat_12, LV_TEXT_ALIGN_LEFT, LV_PCT(100));
    UITheme::muted(status);
    auto* back = Button::create(layout.footer, tr(Text::Back), onBack, this, 160, 44,
                                LV_ALIGN_CENTER, 0, 0, LV_SYMBOL_LEFT);
    UITheme::secondary(back);
}

void StatisticsView::show(const StatisticsSnapshot& value, const std::string& gameTitle) {
    Label::setText(title, tr(Text::Statistics));
    Label::setText(subtitle, gameTitle.empty() ? tr(Text::GlobalStatistics) : gameTitle.c_str());
    Label::setText(hint, tr(Text::StatisticsHint));
    const auto& s = value.totals;
    const std::string values[] = {number(s.played),
                                  number(s.wins),
                                  number(s.losses),
                                  number(s.abandoned),
                                  number(value.active),
                                  s.finished() ? duration(s.averageMs()) : "-",
                                  s.wins ? duration(s.bestMs) : "-",
                                  s.wins ? duration(s.worstMs) : "-",
                                  number(s.players),
                                  s.played ? number(s.lastPlayers) : "-"};
    for (unsigned i = 0; i < 10; ++i) {
        StatisticRow::setTitle(rows[i], tr(keys[i]));
        StatisticRow::setValue(rows[i], values[i].c_str());
    }
    Label::setText(status, tr(!value.persistent ? Text::StatsTemporary
                              : !s.played       ? Text::NoStatistics
                                                : Text::StatisticsSaved));
}

void StatisticsView::onBack(lv_event_t* event) {
    static_cast<StatisticsView*>(lv_event_get_user_data(event))
        ->emitEvent(ButtonClickedEvent("back"));
}
