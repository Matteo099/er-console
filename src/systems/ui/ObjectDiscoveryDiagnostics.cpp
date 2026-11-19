#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include "ObjectDiscoveryDiagnostics.h"
#include "scenes/game/GameScene.h"
#include "systems/lua/bindings/GameSceneBindings.h"
#include "systems/ui/UIFeedback.h"
#include <Arduino.h>
#include <memory>

namespace {
using Object = GameObjectSubview::ObjectData;

Object object(const char* id, const char* title, const char* image = "") {
    Object value;
    value.id = id;
    value.title = title;
    value.image = image;
    value.content.push_back(GameObjectSubview::ContentBlock::textBlock(title));
    return value;
}
class Preview;
Preview* preview = nullptr;

class Preview : public Scene {
  public:
    GameView view;
    unsigned mode;

    explicit Preview(unsigned value) : mode(value) {
    }

    void onEnter() override {
        preview = this;
        view.setEventHandler(this);
        view.create();
        view.showMain();
        view.setTime(240);
        view.setRunning(true);
        view.discoverGameObject(
            object("map", "Mappa / Map",
                   mode == 3 ? "/not-present.png" : "/games/5-t_gameobject/images/map.png"));
        if (mode) {
            view.updateDiscoveryForDiagnostics(1000);
            view.updateDiscoveryForDiagnostics(1000 + (mode == 2 ? 900 : 200));
        }
    }

    void onUpdate() override {
        if (!mode)
            view.update();
    }

    void onExit() override {
        if (preview == this)
            preview = nullptr;
        view.destroy();
    }

    View& getView() override {
        return view;
    }

    bool handleUIEvent(const UIEvent& event) override {
        if (event.getType() != ButtonClickedEvent::TYPE || view.discoveryVisible())
            return false;
        const auto& id = static_cast<const ButtonClickedEvent&>(event).buttonId;
        if (id == "objects")
            view.showGameObject();
        else if (id == "back_main")
            view.showMain();
        return true;
    }
};
}  // namespace

Scene* createObjectDiscoveryPreview(unsigned mode) {
    return new Preview(mode);
}

void objectDiscoveryStatus() {
    auto* view = preview
                     ? &preview->view
                     : (GameScene::diagnosticCurrent ? GameScene::diagnosticCurrent->getGameView()
                                                     : nullptr);
    if (view)
        Serial.printf("@DISCOVERY count=%u active=%u\n", unsigned(view->objectCount()),
                      view->discoveryVisible());
    else
        Serial.println("@DISCOVERY missing-view");
}

void addObjectDiscoveryPreview() {
    if (preview)
        preview->view.discoverGameObject(
            object("board", "Tavola / Board", "/games/5-t_gameobject/images/tavola.png"));
}

void runObjectDiscoveryDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool value, const char* label) {
        value ? ++passed : ++failed;
        Serial.printf("@DISCOVERYCHECK %s %s\n", value ? "PASS" : "FAIL", label);
    };
    auto map = object("map", "Map"), board = object("board", "Board");
    GameObjectInventory inventory;
    inventory.set({map, board});
    check(inventory.count() == 0 && inventory.next()->id == "map",
          "new objects remain hidden in authored order");
    inventory.collect(GameObjectInventory::key(map));
    check(inventory.count() == 1 && inventory.next()->id == "board",
          "landing exposes exactly one object");
    map.title = "Mappa";
    inventory.set({map, board, board});
    check(inventory.count() == 1 && inventory.collected()[0].title == "Mappa",
          "localized stable IDs update without rediscovery");
    inventory.collect(GameObjectInventory::key(board));
    check(inventory.count() == 2 && !inventory.next(),
          "duplicate IDs never create duplicate rewards");
    inventory.set({board});
    check(inventory.count() == 1 && !inventory.contains(GameObjectInventory::key(map)),
          "replacement removes objects explicitly");
    inventory.clear();
    auto unnamed = object("", "Clue");
    inventory.discover(unnamed);
    inventory.discover(unnamed);
    inventory.collect(GameObjectInventory::key(unnamed));
    check(inventory.count() == 1, "objects without IDs use their title as identity");

    const auto sound = UIFeedback::soundEnabled();
    UIFeedback::setSoundEnabled(false);
    auto* original = lv_display_get_default();
    auto* display = lv_display_create(320, 240);
    lv_display_set_default(display);
    {
        std::unique_ptr<GameView> owner(new GameView());
        auto& view = *owner;
        view.create();
        lv_screen_load(view.getScreen());
        view.showMain();
        view.setTime(240);
        view.setGameObjectData({map, board});
        view.updateDiscoveryForDiagnostics(0);
        check(!view.discoveryVisible() && view.objectCount() == 0,
              "ready games defer discoveries until play starts");
        view.setRunning(true);
        view.updateDiscoveryForDiagnostics(100);
        check(view.discoveryVisible() && view.objectCount() == 0,
              "reveal starts before inventory publication");
        view.updateDiscoveryForDiagnostics(1249);
        check(view.objectCount() == 0 && view.discoveryVisible(),
              "object is unavailable throughout the flight");
        view.updateDiscoveryForDiagnostics(1250);
        check(view.objectCount() == 1 && !view.discoveryVisible(),
              "object becomes available at exact landing time");
        AudioPreferences audio;
        view.showSettings(audio, Text::GameSettingsRunning);
        view.updateDiscoveryForDiagnostics(1300);
        check(!view.discoveryVisible(), "settings defer queued discoveries");
        view.hideSettings();
        view.showGameObject();
        view.updateDiscoveryForDiagnostics(1400);
        check(!view.discoveryVisible() && view.objectCount() == 1,
              "reading an object defers the next reveal");
        view.showMain();
        view.updateDiscoveryForDiagnostics(1500);
        view.updateDiscoveryForDiagnostics(2650);
        check(view.objectCount() == 2, "queued discoveries resume on returning to the timer");
        view.discoverGameObject(map);
        view.updateDiscoveryForDiagnostics(2700);
        check(!view.discoveryVisible() && view.objectCount() == 2,
              "updating a collected object does not animate again");
        view.discoverGameObject(object("key", "Key", "/not-present.png"));
        view.updateDiscoveryForDiagnostics(2800);
        view.setRunning(false);
        check(!view.discoveryVisible() && view.objectCount() == 2,
              "pausing interrupts a reveal without publishing an item");
        view.setRunning(true);
        view.updateDiscoveryForDiagnostics(2900);
        view.showExitConfirmation();
        view.updateDiscoveryForDiagnostics(4100);
        check(!view.discoveryVisible() && view.objectCount() == 2,
              "exit confirmation cannot be obscured by a reveal");
        view.hideExitConfirmation();
        view.updateDiscoveryForDiagnostics(4200);
        view.setGameObjectData({map, board});
        check(!view.discoveryVisible() && view.objectCount() == 2,
              "removing an in-flight item cancels its landing");
        view.discoverGameObject(object("last", "Last"));
        view.updateDiscoveryForDiagnostics(4300);
        view.showFailure();
        check(!view.discoveryVisible() && view.objectCount() == 2,
              "game over dismisses discovery immediately");
        view.showLoading();
        check(view.objectCount() == 0 && !view.discoveryVisible(),
              "retry clears collected and pending objects");
        view.destroy();
    }
    {
        std::unique_ptr<GameScene> scene(new GameScene());
        GameInfo info;
        info.id = "5-t_gameobject";
        scene->setGameInfo(info);
        auto& view = *scene->getGameView();
        view.create();
        lv_screen_load(view.getScreen());
        view.showMain();
        view.setTime(240);
        view.setRunning(true);
        auto* vm = luaL_newstate();
        GameSceneBindings binding(scene.get());
        if (vm)
            binding.registerAPI(vm);
        const bool okay = vm && luaL_dostring(vm, R"lua(
            scene.setGameObjects({{id='map',title='Map',content={{type='text',value='first'}}}})
            scene.discoverGameObject({id='board',title='Board',image='images/tavola.png'})
            scene.discoverGameObject({id='board',title='Updated Board'})
        )lua") == LUA_OK;
        scene->applyObjectsForDiagnostics();
        view.updateDiscoveryForDiagnostics(100);
        view.updateDiscoveryForDiagnostics(1250);
        view.updateDiscoveryForDiagnostics(1300);
        view.updateDiscoveryForDiagnostics(2450);
        check(okay && view.objectCount() == 2,
              "Lua snapshot plus rapid incremental discoveries preserve both unique items");
        const bool badType = vm && luaL_dostring(vm, "scene.discoverGameObject(42)") != LUA_OK;
        check(badType, "Lua rejects non-table discoveries without touching the view");
        if (vm)
            lua_close(vm);
        view.destroy();
    }
    lv_display_delete(display);
    lv_display_set_default(original);
    UIFeedback::setSoundEnabled(sound);
    Serial.printf("@DISCOVERYTEST passed=%u failed=%u stack=%u\n", passed, failed,
                  unsigned(uxTaskGetStackHighWaterMark(nullptr)));
}
#endif
