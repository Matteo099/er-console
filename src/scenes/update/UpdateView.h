#pragma once
#include "systems/ui/View.h"
#include "systems/update/UpdateTypes.h"
#include "systems/ui/components/TextInputDialog.h"
#include "systems/ui/components/ConfirmationDialog.h"

class UpdateView : public View
{
public:
    void create() override;
    void update() override {}
    void destroy() override;
    lv_obj_t *getScreen() override { return screen; }
    void render(const UpdateSnapshot &snapshot);
    void askInstall(const std::string &version);
    void editToken();
    void showTokenError();

private:
    static void onButton(lv_event_t *event);
    lv_obj_t *screen = nullptr, *title = nullptr, *statusLabel = nullptr, *progressBar = nullptr, *transferLabel = nullptr;
    lv_obj_t *installed = nullptr, *available = nullptr, *releaseTitle = nullptr, *notes = nullptr, *tokenStatus = nullptr;
    lv_obj_t *checkButton = nullptr, *installButton = nullptr, *backButton = nullptr, *tokenButton = nullptr, *clearButton = nullptr, *networkButton = nullptr;
    TextInputDialog editor;
    ConfirmationDialog confirmation;
};
