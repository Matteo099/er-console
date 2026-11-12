#include "systems/ui/localization/I18n.h"
#include "BootView.h"
#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"

void BootView::create() {
    screen = Screen::create();
    auto layout = SceneLayout::create(screen, 44, 64, false);
    topRow = layout.top;
    middleRow = layout.body;
    bottomRow = layout.footer;
    brandImg = BrandHeader::create(topRow);
    auto* title =
        Label::create(middleRow, tr(Text::LetsGo), UITheme::heading(26), LV_TEXT_ALIGN_CENTER);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -15);
    auto* hint = Label::create(middleRow, tr(Text::PrepareAdventure), &lv_font_montserrat_12);
    UITheme::muted(hint);
    lv_obj_align(hint, LV_ALIGN_CENTER, 0, 19);
    progressBar = lv_bar_create(bottomRow);
    lv_obj_set_size(progressBar, LV_PCT(86), 8);
    lv_obj_align(progressBar, LV_ALIGN_TOP_MID, 0, 8);
    statusLabel = Label::create(bottomRow, tr(Text::Starting), &lv_font_montserrat_12,
                                LV_TEXT_ALIGN_CENTER, LV_PCT(95));
    UITheme::muted(statusLabel);
    lv_obj_align(statusLabel, LV_ALIGN_BOTTOM_MID, 0, -16);
}

void BootView::setProgress(uint8_t progress, const char*) {
    if (!screen)
        return;
    lv_bar_set_value(progressBar, progress, LV_ANIM_OFF);
    const char* step = progress >= 100  ? tr(Text::AllReady)
                       : progress >= 87 ? tr(Text::BootLua)
                       : progress >= 75 ? tr(Text::BootInput)
                       : progress >= 62 ? tr(Text::BootAudio)
                       : progress >= 50 ? tr(Text::BootNetwork)
                       : progress >= 37 ? tr(Text::BootStorage)
                       : progress >= 25 ? tr(Text::BootDisplay)
                       : progress >= 12 ? tr(Text::BootLed)
                                        : tr(Text::BootCore);
    char text[64];
    lv_snprintf(text, sizeof(text), "%s  %u%%", step, progress);
    Label::setText(statusLabel, text);
}

void BootView::update() {
}

void BootView::destroy() {
    screen = topRow = middleRow = bottomRow = brandImg = statusLabel = progressBar = nullptr;
}
