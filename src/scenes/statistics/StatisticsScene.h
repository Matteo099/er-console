#pragma once
#include "systems/ui/Scene.h"
#include "StatisticsView.h"
#include "scenes/game/GameInfo.h"
#include "systems/ui/UIEvents.h"

class StatisticsScene : public Scene
{
public:
    StatisticsScene() = default;
    explicit StatisticsScene(const GameInfo &game) : game(game),perGame(true) {}
    void onEnter() override { view.setEventHandler(this); view.create(); refresh(); }
    void onUpdate() override {}
    void onExit() override { view.destroy(); }
    void onPreferencesChanged() override { refresh(); }
    View &getView() override { return view; }
    SceneRequest getRequest() const override { return request; }
    bool handleUIEvent(const UIEvent &event) override
    {
        if (event.getType()!=ButtonClickedEvent::TYPE || static_cast<const ButtonClickedEvent &>(event).buttonId!="back") return false;
        request=SceneRequest(perGame ? SceneChange::GoToGameList : SceneChange::GoToSettingsSystem,SceneTransition::SlideRight,perGame ? &game:nullptr); return true;
    }
private:
    void refresh() { view.show(StatisticsRepository::instance().snapshot(perGame ? game.id:""),perGame ? game.titleFor(I18n::language()==I18n::Language::English):""); }
    GameInfo game;
    bool perGame=false;
    StatisticsView view;
    SceneRequest request;
};
