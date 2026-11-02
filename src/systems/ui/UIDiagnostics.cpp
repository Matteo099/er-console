#include "UIDiagnostics.h"
#include "config/Console.h"

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include <cstring>
#include "systems/ui/SceneManager.h"
#include "systems/ui/UITheme.h"
#include "scenes/menu/MainMenuScene.h"
#include "scenes/settings/SettingsScene.h"
#include "scenes/gamelist/GameListScene.h"
#include "scenes/monitor/MonitorScene.h"
#include "scenes/about/AboutScene.h"
#include "scenes/game/GameView.h"
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include "drivers/bus/SPIBusLock.h"
#include "config/Assets.h"

namespace
{
    // Preview game views without executing Lua, touching relays or starting a game.
    class GamePreview : public Scene
    {
        GameView view;
        unsigned mode;
    public:
        explicit GamePreview(unsigned value) : mode(value) {}
        void onEnter() override
        {
            view.create();
            view.setTime(3600);
            if (mode == 1)
            {
                VictorySubview::GameStats stats;
                stats.elapsedSeconds = 1800; stats.pausedSeconds = 60;
                stats.hintsUsed = 2; stats.puzzlesSolved = 6;
                view.setVictoryStats(stats); view.showVictory();
            }
            else if (mode == 2) view.showFailure();
            else if (mode == 3)
            {
                GameObjectSubview::ObjectData object;
                object.title = "La mappa del laboratorio";
                object.content.push_back(GameObjectSubview::ContentBlock::textBlock("Esamina gli indizi per trovare la prossima combinazione."));
                object.content.push_back(GameObjectSubview::ContentBlock::imageBlock("/games/5-t_gameobject/images/tavola.png"));
                view.setGameObjectData({object}); view.showGameObject();
            }
            else { view.showMain(); view.setRunning(false); }
        }
        void onUpdate() override { view.update(); }
        void onExit() override { view.destroy(); }
        View &getView() override { return view; }
    };

    lv_obj_t *buttonAt(lv_obj_t *obj, int x, int y)
    {
        if (!lv_obj_is_visible(obj)) return nullptr;
        lv_area_t area;
        lv_obj_get_coords(obj, &area);
        if (x < area.x1 || x > area.x2 || y < area.y1 || y > area.y2) return nullptr;
        for (int i = int(lv_obj_get_child_count(obj)) - 1; i >= 0; --i)
            if (auto *found = buttonAt(lv_obj_get_child(obj, i), x, y)) return found;
        return lv_obj_check_type(obj, &lv_button_class) ? obj : nullptr;
    }

    void tree(lv_obj_t *obj)
    {
        if (!lv_obj_is_visible(obj)) return;
        lv_area_t a;
        lv_obj_get_coords(obj, &a);
        if (lv_obj_check_type(obj, &lv_button_class))
        {
            Serial.printf("@BUTTON %d %d %d %d disabled=%d ", a.x1, a.y1, a.x2, a.y2,
                          lv_obj_has_state(obj, LV_STATE_DISABLED));
            for (uint32_t i = 0; i < lv_obj_get_child_count(obj); ++i)
            {
                auto *child = lv_obj_get_child(obj, i);
                if (lv_obj_check_type(child, &lv_label_class)) Serial.printf("%s ", lv_label_get_text(child));
            }
            Serial.println();
        }
        if (lv_obj_check_type(obj, &lv_label_class))
        {
            String text = lv_label_get_text(obj);
            text.replace("\n", " / ");
            Serial.printf("@LABEL %d %d %d %d %s\n", a.x1, a.y1, a.x2, a.y2, text.c_str());
        }
        for (uint32_t i = 0; i < lv_obj_get_child_count(obj); ++i) tree(lv_obj_get_child(obj, i));
    }

    void snapshot()
    {
        auto *buffer = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_RGB565);
        if (!buffer) { Serial.println("@SNAPSHOT ERROR allocation"); return; }
        const unsigned width = buffer->header.w, height = buffer->header.h;
        Serial.printf("@SNAPSHOT BEGIN %u %u\n", width, height);
        char line[205];
        unsigned used = 4;
        std::memcpy(line, "@PX ", 4);
        const char hex[] = "0123456789abcdef";
        auto append = [&](uint8_t byte)
        {
            line[used++] = hex[byte >> 4];
            line[used++] = hex[byte & 15];
            if (used == 196)
            {
                line[used++] = '\n';
                Serial.write(reinterpret_cast<uint8_t *>(line), used);
                used = 4;
            }
        };
        auto pixel = [&](unsigned index)
        {
            const auto *p = buffer->data + (index / width) * buffer->header.stride + (index % width) * 2;
            return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
        };
        for (unsigned i = 0; i < width * height;)
        {
            const uint16_t color = pixel(i);
            unsigned count = 1;
            while (count < 255 && i + count < width * height && pixel(i + count) == color) ++count;
            append(count); append(color & 255); append(color >> 8);
            i += count;
        }
        if (used > 4) { line[used++] = '\n'; Serial.write(reinterpret_cast<uint8_t *>(line), used); }
        Serial.println("@SNAPSHOT END");
        lv_draw_buf_destroy(buffer);
    }

    void checkStorage()
    {
        auto *storage = static_cast<StorageSystem *>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
        if (!storage || !storage->isReady()) { Serial.println("@SDCHECK unavailable"); return; }
        SPIBusLock::Guard guard;
        for (const char *path : {IMG_BRAND_NEGATIVE_FILE, IMG_LOGO_FILE})
            for (unsigned pass = 1; pass <= 2; ++pass)
            {
                auto file = storage->openNative(path, false);
                if (!file) { Serial.printf("@SDCHECK missing %s\n", path); continue; }
                const size_t expected = file.size();
                uint8_t buffer[512];
                uint32_t hash = 2166136261u;
                size_t total = 0;
                const uint32_t started = millis();
                while (total < expected)
                {
                    const size_t count = file.read(buffer, min(sizeof(buffer), expected - total));
                    if (!count || count > sizeof(buffer)) break;
                    for (size_t i = 0; i < count; ++i) hash = (hash ^ buffer[i]) * 16777619u;
                    total += count;
                }
                const uint32_t elapsed = millis() - started;
                file.close();
                Serial.printf("@SDCHECK %s pass=%u bytes=%u/%u ms=%u fnv1a=%08lx\n", path, pass,
                              (unsigned)total, (unsigned)expected, (unsigned)elapsed, (unsigned long)hash);
            }
    }
}

void UIDiagnostics::update(SceneManager &scenes)
{
    static String command;
    while (Serial.available())
    {
        const char value = Serial.read();
        if (value != '\n') { if (value != '\r' && command.length() < 96) command += value; continue; }
        if (command == "ui home") scenes.changeScene(new MainMenuScene(), SceneTransition::None);
        else if (command == "ui settings") scenes.changeScene(new SettingsScene(), SceneTransition::None);
        else if (command == "ui games") scenes.changeScene(new GameListScene(), SceneTransition::None);
        else if (command == "ui monitor") scenes.changeScene(new MonitorScene(), SceneTransition::None);
        else if (command == "ui about") scenes.changeScene(new AboutScene(), SceneTransition::None);
        else if (command == "ui game") scenes.changeScene(new GamePreview(0), SceneTransition::None);
        else if (command == "ui victory") scenes.changeScene(new GamePreview(1), SceneTransition::None);
        else if (command == "ui fail") scenes.changeScene(new GamePreview(2), SceneTransition::None);
        else if (command == "ui objects") scenes.changeScene(new GamePreview(3), SceneTransition::None);
        else if (command == "ui snapshot") snapshot();
        else if (command == "ui tree") tree(lv_screen_active());
        else if (command == "ui sdcheck") checkStorage();
        else if (command.startsWith("ui theme ")) UITheme::select(UITheme::fromName(command.substring(9).c_str()));
        else if (command.startsWith("ui click "))
        {
            int x = 0, y = 0;
            if (sscanf(command.c_str(), "ui click %d %d", &x, &y) == 2)
                if (auto *button = buttonAt(lv_screen_active(), x, y))
                    if (!lv_obj_has_state(button, LV_STATE_DISABLED)) lv_obj_send_event(button, LV_EVENT_CLICKED, nullptr);
        }
        else if (command.startsWith("ui scroll "))
        {
            int y = 0;
            sscanf(command.c_str(), "ui scroll %d", &y);
            // SceneLayout's body is the second child. Settings panels scroll independently.
            if (auto *body = lv_obj_get_child(lv_screen_active(), 1)) lv_obj_scroll_to_y(body, y, LV_ANIM_OFF);
        }
        Serial.printf("@UI DONE %s\n", command.c_str());
        command = "";
    }
}
#else
void UIDiagnostics::update(SceneManager &) {}
#endif
