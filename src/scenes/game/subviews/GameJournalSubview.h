#pragma once
#include "GameObjectSubview.h"
#include <functional>

// Navigation contains only unlocked titles, never hidden hint definitions.
class GameJournalSubview {
  public:
    void create(lv_obj_t* parent, EventCallback back, std::function<void(bool, size_t)> selected);
    void setLists(const std::vector<GameObjectSubview::ObjectData>& objects,
                  const std::vector<GameObjectSubview::ObjectData>& hints);
    void show(bool hints);
    void hide();
    void destroy();
    void translate();

    bool hintsSelected() const {
        return hintsTab;
    }

    lv_obj_t* getControls() const {
        return controls;
    }

    lv_obj_t* getTab(bool hints) const {
        return hints ? hintButton : objectButton;
    }

  private:
    lv_obj_t *root = nullptr, *body = nullptr, *controls = nullptr;
    lv_obj_t *objectButton = nullptr, *hintButton = nullptr, *backButton = nullptr;
    std::vector<std::string> objectTitles, hintTitles;
    bool hintsTab = false;
    EventCallback back;
    std::function<void(bool, size_t)> selected;
    void rebuild();
    static void onTab(lv_event_t* event);
    static void onEntry(lv_event_t* event);
    static void onBack(lv_event_t* event);
};
