#pragma once

#include "systems/ui/View.h"
#include "systems/ui/UIEventSystem.h"

enum class SceneTransition
{
    None,
    SlideLeft,
    SlideRight,
    Fade
};

enum class SceneChange
{
    None,
    GoToMainMenu,
    GoToGameList,
    GoToSettings,
    GoToMonitor,
    GoToAbout,
    GoToNetwork,
    GoToGame,
};

struct SceneRequest
{
    SceneChange change;
    void* params;

    // Default (no-op)
    constexpr SceneRequest()
        : change(SceneChange::None), params(nullptr)
    {}

    // Explicit request
    constexpr SceneRequest(SceneChange c, void* p = nullptr)
        : change(c), params(p)
    {}

    constexpr bool isNone() const
    {
        return change == SceneChange::None;
    }
};

class Scene : public UIEventHandler
{
public:
    virtual ~Scene() = default;

    virtual void onEnter() = 0;
    virtual void onUpdate() = 0;
    virtual void onExit() = 0;

    virtual View &getView() = 0;
    virtual SceneTransition getTransition() const { return SceneTransition::SlideLeft; }
    virtual SceneRequest getRequest() const
    {
        return {}; // default SceneRequest (None, nullptr)
    }

    // UIEventHandler implementation - scenes can override to handle events
    bool handleUIEvent(const UIEvent& event) override
    {
        // Default implementation does nothing - scenes can override
        return false;
    }
};
