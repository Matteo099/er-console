#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include "HintDiagnostics.h"
#include "scenes/game/GameScene.h"
#include "systems/lua/bindings/GameSceneBindings.h"
#include "systems/ui/UIFeedback.h"
#include <Arduino.h>
#include <memory>
#include <cstring>

namespace {
using Data = GameObjectSubview::ObjectData;

Data hint(const char* id, const char* title) {
    Data data;
    data.id = id;
    data.title = title;
    data.content.push_back(GameObjectSubview::ContentBlock::textBlock(
        I18n::language() == I18n::Language::Italian
            ? "Osserva i simboli e confrontali con la mappa."
            : "Look at the symbols and compare them with the map."));
    return data;
}

HintDefinition rule(const char* id, const char* step, uint32_t delay) {
    HintDefinition value;
    value.data = hint(id, id);
    value.step = step;
    value.afterMs = delay;
    return value;
}

lv_obj_t* button(lv_obj_t* root, const char* caption) {
    if (lv_obj_has_flag(root, LV_OBJ_FLAG_HIDDEN))
        return nullptr;
    if (lv_obj_check_type(root, &lv_label_class) && !strcmp(lv_label_get_text(root), caption) &&
        lv_obj_check_type(lv_obj_get_parent(root), &lv_button_class))
        return lv_obj_get_parent(root);
    for (unsigned i = 0; i < lv_obj_get_child_count(root); ++i)
        if (auto* result = button(lv_obj_get_child(root, i), caption))
            return result;
    return nullptr;
}
class Preview;
Preview* active = nullptr;

class Preview : public Scene {
  public:
    std::unique_ptr<GameScene> scene;
    uint32_t started = 0;
    unsigned mode;

    explicit Preview(unsigned value) : mode(value) {
    }

    void onEnter() override {
        active = this;
        scene.reset(new GameScene());
        GameInfo info;
        info.id = "5-t_gameobject";
        scene->setGameInfo(info);
        auto& view = *scene->getGameView();
        view.setEventHandler(this);
        view.create();
        view.showMain();
        view.setTime(240);
        const bool english = I18n::language() == I18n::Language::English;
        auto first = rule("first", "door", 1000);
        first.data.title = english ? "Inspect the board" : "Osserva la tavola";
        first.data.image = "/games/5-t_gameobject/images/tavola.png";
        first.data.content.push_back(
            GameObjectSubview::ContentBlock::imageBlock(first.data.image, 240, 160));
        scene->postHints({first});
        scene->postGameStep("door");
        scene->applyObjectsForDiagnostics();
        started = millis();
        view.setRunning(mode == 0);
        if (mode == 1)
            view.showJournal(true);
        if (mode == 2) {
            for (unsigned i = 0; i < 8; ++i) {
                const auto title =
                    std::string(english ? "Hint " : "Indizio ") + std::to_string(i + 1);
                scene->postHint(hint(("manual-" + std::to_string(i)).c_str(), title.c_str()));
            }
            scene->applyObjectsForDiagnostics();
            view.showJournal(true);
        }
    }

    void onUpdate() override {
        if (!mode)
            scene->updateHintsForDiagnostics(millis() - started);
        scene->getGameView()->update();
    }

    void onExit() override {
        if (active == this)
            active = nullptr;
        scene->getGameView()->destroy();
    }

    View& getView() override {
        return *scene->getGameView();
    }

    bool handleUIEvent(const UIEvent& event) override {
        if (event.getType() == HintViewedEvent::TYPE)
            return scene->handleUIEvent(event);
        if (event.getType() != ButtonClickedEvent::TYPE)
            return false;
        auto& view = *scene->getGameView();
        const auto& id = static_cast<const ButtonClickedEvent&>(event).buttonId;
        if (id == "objects")
            view.showJournal();
        else if (id == "back_main")
            view.showJournal(view.hintsSelected());
        else if (id == "journal_back")
            view.showMain();
        return true;
    }
};
}  // namespace

Scene* createHintPreview(unsigned mode) {
    return new Preview(mode);
}

void hintPreviewStatus() {
    if (!active) {
        Serial.println("@HINTS no-preview");
        return;
    }
    auto& view = *active->scene->getGameView();
    Serial.printf("@HINTS count=%u active=%u state=%u reading=%u viewed=%u\n",
                  unsigned(view.hintCount()), unsigned(view.discoveryVisible()),
                  unsigned(view.currentSubview()), unsigned(view.hintsSelected()),
                  unsigned(active->scene->viewedHintsForDiagnostics()));
}

void runHintDiagnostics() {
    unsigned passed = 0, failed = 0;
    auto check = [&](bool value, const char* name) {
        value ? ++passed : ++failed;
        Serial.printf("@HINTCHECK %s %s\n", value ? "PASS" : "FAIL", name);
    };
    GameHints schedule;
    schedule.configure(
        {rule("second", "A", 4000), rule("first", "A", 2000), rule("B-hint", "B", 1000)});
    check(!schedule.update(9000), "no hints unlock before a room step is selected");
    schedule.setStep("A", 0);
    check(!schedule.update(1999), "hints stay locked before their exact deadline");
    check(schedule.update(2000) && schedule.available()[0].id == "first",
          "only a due hint unlocks at its deadline");
    check(!schedule.update(2000) && schedule.available().size() == 1, "a hint unlocks once");
    schedule.setStep("A", 2500);
    check(schedule.update(4000) && schedule.available()[1].id == "second",
          "repeating a step does not restart its clock");
    schedule.setStep("B", 5000);
    check(!schedule.update(5999) && schedule.update(6000),
          "a new step starts a fresh active-play deadline");
    check(schedule.available().size() == 3, "previous hints remain accessible after step changes");
    auto translated = rule("first", "A", 2000);
    translated.data.title = "Tradotto";
    schedule.configure({translated});
    check(schedule.available().size() == 3 && schedule.available()[0].title == "Tradotto",
          "localized definitions update without relocking old hints");
    check(schedule.markViewed("first") && !schedule.markViewed("first") &&
              !schedule.markViewed("unknown") && schedule.viewedCount() == 1,
          "hint usage counts unique viewed hints, not repeated visits");
    schedule.clear();
    schedule.configure({rule("skipped", "A", 2000)});
    schedule.setStep("A", 0);
    schedule.setStep("done", 1500);
    check(!schedule.update(9000) && schedule.available().empty(),
          "reaching the next step cancels previous deadlines");
    schedule.clear();
    schedule.configure({rule("paused", "A", 2000)});
    schedule.setStep("A", 0);
    GameSessionClock clock;
    clock.start(100);
    clock.pause(1100);
    check(!schedule.update(clock.elapsed(9000)), "pause time does not unlock hints");
    clock.resume(9100);
    check(schedule.update(clock.elapsed(10100)), "the deadline resumes with active play");
    schedule.clear();
    schedule.configure({rule("late", "A", 4000), rule("early", "A", 1000)});
    schedule.setStep("A", 0);
    schedule.update(10000);
    check(schedule.available()[0].id == "early" && schedule.available()[1].id == "late",
          "missed deadlines unlock in increasing time order");

    const auto sound = UIFeedback::soundEnabled();
    UIFeedback::setSoundEnabled(false);
    auto* original = lv_display_get_default();
    auto* display = lv_display_create(320, 240);
    lv_display_set_default(display);
    {
        std::unique_ptr<GameScene> scene(new GameScene());
        GameInfo info;
        info.id = "5-t_gameobject";
        scene->setGameInfo(info);
        auto& view = *scene->getGameView();
        view.setEventHandler(scene.get());
        view.create();
        lv_screen_load(view.getScreen());
        view.showMain();
        view.setTime(240);
        view.setRunning(true);
        auto* vm = luaL_newstate();
        if (vm) {
            luaL_openlibs(vm);
            GameSceneBindings(scene.get()).registerAPI(vm);
        }
        const bool valid = vm && luaL_dostring(vm, R"lua(
            scene.setHints({{id='door',step='find-door',afterSeconds=2,title='Door hint',
                content={{type='text',value='Look by the door.'},{type='image',path='images/map.png',width=120,height=80}}}})
            scene.setStep('find-door')
        )lua") == LUA_OK;
        scene->applyObjectsForDiagnostics();
        scene->updateHintsForDiagnostics(1999);
        check(valid && !view.hintCount(), "Lua-configured rich hints are hidden until due");
        scene->updateHintsForDiagnostics(2000);
        check(view.hintCount() == 1 && !view.discoveryVisible(),
              "unlocked hints enter the list before their notification");
        view.updateDiscoveryForDiagnostics(100);
        check(view.discoveryVisible(), "a new hint starts the discovery animation");
        view.updateDiscoveryForDiagnostics(1250);
        check(view.currentSubview() == GameSubview::GameObject && view.hintsSelected() &&
                  scene->viewedHintsForDiagnostics() == 1,
              "landing automatically opens the new hint and records its first view");
        view.showJournal(true);
        lv_obj_update_layout(view.getScreen());
        auto* row = button(view.getScreen(), "Door hint");
        if (row)
            lv_obj_send_event(row, LV_EVENT_CLICKED, nullptr);
        check(row && scene->viewedHintsForDiagnostics() == 1,
              "previously unlocked hints reopen from the list without double counting");
        if (vm)
            luaL_dostring(vm, "scene.unlockHint({id='other',title='Second "
                              "hint',content={{type='text',value='Second'}}})");
        scene->applyObjectsForDiagnostics();
        view.updateDiscoveryForDiagnostics(1400);
        check(view.hintCount() == 2 && !view.discoveryVisible(),
              "reading a hint defers the next popup while retaining it in the list");
        view.showJournal(true);
        view.updateDiscoveryForDiagnostics(1500);
        view.updateDiscoveryForDiagnostics(2650);
        check(view.hintsSelected() && scene->viewedHintsForDiagnostics() == 2,
              "queued hint opens after leaving the previous reader");
        view.showLoading(false);
        view.showMain();
        view.setRunning(true);
        check(view.hintCount() == 2, "hardware start/resume checks preserve the hint collection");
        AudioPreferences audio;
        view.showSettings(audio, Text::GameSettingsRunning);
        if (vm)
            luaL_dostring(vm, "scene.unlockHint({id='settings',title='Settings hint'})");
        scene->applyObjectsForDiagnostics();
        view.updateDiscoveryForDiagnostics(2700);
        check(!view.discoveryVisible() && view.hintCount() == 3,
              "settings postpone automatic display without hiding unlocked hints");
        view.hideSettings();
        view.setRunning(false);
        view.showJournal(true);
        lv_obj_update_layout(view.getScreen());
        row = button(view.getScreen(), "Settings hint");
        if (row) {
            lv_obj_scroll_to_view(row, LV_ANIM_OFF);
            lv_obj_update_layout(view.getScreen());
            lv_obj_send_event(row, LV_EVENT_CLICKED, nullptr);
        }
        view.setRunning(true);
        view.showMain();
        view.updateDiscoveryForDiagnostics(2800);
        check(!view.discoveryVisible() && scene->viewedHintsForDiagnostics() == 3,
              "manual reading while paused consumes its pending notification");
        for (const char* invalid :
             {"scene.setHints(42)", "scene.setHints({42})",
              "scene.setHints({{title='Missing ID',step='A',afterSeconds=1}})",
              "scene.setHints({{id='x',title='X',step='A',afterSeconds=-1}})",
              "scene.setHints({{id='x',title='X',step='A',afterSeconds=0/0}})",
              "scene.setHints({{id='x',title='X',step='A',afterSeconds=1/0}})",
              "scene.setHints({{id='x',title='X',step='',afterSeconds=1}})",
              "scene.setHints({{id='x',title='X',step='A',afterSeconds=1},{id='x',title='Y',step='"
              "B',afterSeconds=2}})",
              "scene.setStep(1)", "scene.unlockHint({title='No ID'})"})
            check(vm && luaL_dostring(vm, invalid) != LUA_OK,
                  "invalid Lua hint definitions fail before changing configuration");
        if (vm)
            lua_close(vm);
        view.showLoading();
        check(!view.hintCount(), "retry clears hints and pending announcements");
        view.destroy();
    }
    lv_display_delete(display);
    lv_display_set_default(original);
    UIFeedback::setSoundEnabled(sound);
    Serial.printf("@HINTTEST passed=%u failed=%u stack=%u\n", passed, failed,
                  unsigned(uxTaskGetStackHighWaterMark(nullptr)));
}
#endif
