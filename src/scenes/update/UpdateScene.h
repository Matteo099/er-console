#pragma once
#include "UpdateView.h"
#include "systems/ui/Scene.h"
#include "systems/ui/UIEvents.h"
#include "systems/update/UpdateSystem.h"

class UpdateScene : public Scene
{
public:
    void onEnter() override
    {
        view.setEventHandler(this);
        view.create();
        refresh();
    }
    void onUpdate() override
    {
        if (revision != UpdateSystem::instance().revision())
            refresh();
    }
    void onExit() override
    {
        UpdateSystem::instance().cancel();
        view.destroy();
    }
    void onPreferencesChanged() override { refresh(); }
    View &getView() override { return view; }
    SceneRequest getRequest() const override { return request; }
    bool handleUIEvent(const UIEvent &event) override
    {
        auto &system = UpdateSystem::instance();
        if (event.getType() != ButtonClickedEvent::TYPE)
            return false;
        const auto &id = static_cast<const ButtonClickedEvent &>(event).buttonId;
        const auto state = system.snapshot();
        if (id == "check")
        {
            if (state.busy())
                system.cancel();
            else
                system.check();
        }
        else if (state.busy())
            return true;
        else if (id == "install")
            view.askInstall(state.release.version);
        else if (id == "confirm_install")
            system.install();
        else if (id == "back")
            request = SceneRequest(SceneChange::GoToSettingsSystem, SceneTransition::SlideRight);
        else if (id == "wifi_settings")
            request = SceneRequest(SceneChange::GoToNetwork, SceneTransition::SlideLeft);
        else
            return false;
        return true;
    }

private:
    void refresh()
    {
        const auto value = UpdateSystem::instance().snapshot();
        revision = value.revision;
        view.render(value);
    }
    UpdateView view;
    SceneRequest request;
    uint32_t revision = UINT32_MAX;
};
