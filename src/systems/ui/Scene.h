#pragma once

#include "systems/ui/View.h"

enum class SceneTransition {
    None,
    SlideLeft,
    Fade
};

enum class SceneRequest {
    None,
    GoToMainMenu
};

class Scene
{
public:
    virtual ~Scene() = default;

    virtual void onEnter() = 0;
    virtual void onUpdate() = 0;
    virtual void onExit() = 0;

    virtual View& getView() = 0;
    virtual SceneTransition getTransition() const { return SceneTransition::SlideLeft; }
    virtual SceneRequest getRequest() const { return SceneRequest::None; }
};
