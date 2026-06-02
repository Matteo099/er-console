#include "BootView.h"
#include "log/Logger.h"
#include "config/Assets.h"

#include "systems/ui/components/BrandHeader.h"
#include "systems/ui/components/ImageView.h"
#include "systems/ui/components/Label.h"
#include "systems/ui/components/ProgressBar.h"
#include "systems/ui/components/SceneLayout.h"
#include "systems/ui/components/Screen.h"
#include "systems/ui/components/Align.h"
#include "systems/ui/components/Spinner.h"

void BootView::create()
{
    createTextOnly();
    loadImages();
}

void BootView::createTextOnly()
{
    screen = Screen::create();

    auto layout = SceneLayout::create(screen, 70, 100);
    topRow = layout.top;
    middleRow = layout.body;
    bottomRow = layout.footer;

    statusLabel = Label::create(bottomRow, "Starting...", &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);
    Label::setTextColor(statusLabel, lv_color_hex(0xCCCCCC));
    Align::setPosition(statusLabel, LV_ALIGN_CENTER, 0, 20);

    progressBar = ProgressBar::create(bottomRow);
    Align::alignTo(progressBar, statusLabel, LV_ALIGN_OUT_TOP_MID, 0, -12);

    brandImg = nullptr;
    logoImg = nullptr;

    currentProgress = 0;
    currentLabel = nullptr;
}

void BootView::loadImages()
{
    if (!screen)
        return;

    Log::debug(Log::SCENE_TAG, "Loading boot images into view");

    if (!topRow)
        return;

    brandImg = BrandHeader::create(topRow);
    logoImg = ImageView::create(middleRow, IMG_LOGO_FILE, LV_ALIGN_CENTER, 0, 0);

    // Start spinning animation on logo
    Spinner::start(logoImg, 600);  // 600ms per rotation
}

void BootView::setProgress(uint8_t progress, const char *label)
{
    currentProgress = progress;
    currentLabel = label;
}

void BootView::update()
{
    if (!screen)
        return;

    if (progressBar)
    {
        ProgressBar::setValue(progressBar, currentProgress, LV_ANIM_ON);
    }

    if (statusLabel && currentLabel)
    {
        lv_label_set_text(statusLabel, currentLabel);
    }
}

void BootView::destroy()
{
    // Clean up tracker (for any dynamic objects)
    getObjectTracker().cleanup();

    // Clean up spinner animation before object deletion
    Spinner::cleanup(logoImg);

    // Delete root (cascades to all children)
    UICleanup::deleteRoot(screen);

    // Nullify remaining pointers
    topRow = nullptr;
    bottomRow = nullptr;
    brandImg = nullptr;
    logoImg = nullptr;
    statusLabel = nullptr;
    progressBar = nullptr;
    currentLabel = nullptr;
    currentProgress = 0;
}
