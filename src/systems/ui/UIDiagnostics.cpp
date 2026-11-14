#include "UIDiagnostics.h"
#include "HardwareDiagnostics.h"
#include "config/Console.h"

#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include <Arduino.h>
#include <cstring>
#include "systems/ui/SceneManager.h"
#include "systems/ui/UITheme.h"
#include "systems/ui/UIFrameStats.h"
#include "systems/ui/UIFeedback.h"
#include "systems/audio/AudioEvent.h"
#include "systems/audio/AudioSystem.h"
#include "services/DeviceConfigRepository.h"
#include "scenes/menu/MainMenuScene.h"
#include "scenes/settings/SettingsScene.h"
#include "scenes/gamelist/GameListScene.h"
#include "scenes/monitor/MonitorScene.h"
#include "scenes/about/AboutScene.h"
#include "scenes/network/NetworkScene.h"
#include "scenes/game/GameView.h"
#include "core/SystemEventBus.h"
#include "systems/storage/StorageSystem.h"
#include "drivers/bus/SPIBusLock.h"
#include "config/Assets.h"
#include "SettingsDiagnostics.h"
#include "ConfigStorageDiagnostics.h"
#include "AudioDiagnostics.h"
#include "ScrollDiagnostics.h"
#include "PlayerSelectorDiagnostics.h"
#include "VolumeFeedbackDiagnostics.h"
#include "AudioPreloadDiagnostics.h"
#include "InGameSettingsDiagnostics.h"
#include "StatisticsDiagnostics.h"
#include "GameAssetDiagnostics.h"
#include "UpdateDiagnostics.h"
#include "scenes/update/UpdateScene.h"
#include "services/StatisticsRepository.h"
#include "scenes/game/GameScene.h"
#include "scenes/statistics/StatisticsScene.h"
#include "drivers/display/TouchController.h"
#include "GameFixtureDiagnostics.h"
#include "NetworkConnectionDiagnostics.h"
#include "services/GameMetadata.h"
#include "systems/ui/UIEvents.h"

namespace {
bool simulatedTouch = false;
TouchPoint touchPoint{0, 0, false};
std::string contentFixture;
class NetworkPreview;
NetworkPreview* activeNetworkPreview = nullptr;

class NetworkPreview : public Scene {
    NetworkView view;
    WifiScanSnapshot scan;

  public:
    explicit NetworkPreview(WifiScanState state, bool empty = false) {
        scan.state = state;
        if (!empty && (state == WifiScanState::Ready || state == WifiScanState::Scanning))
            scan.networks = {{"Studio", -42, true, "wpa2"},
                             {"Laboratorio", -66, true, "wpa2"},
                             {"Guest", -85, false, "open"}};
    }

    void onEnter() override {
        activeNetworkPreview = this;
        view.setEventHandler(this);
        view.create();
        view.setCredentials("Studio", "demo12345");
        view.setStatus(tr(Text::Disconnected), "Studio", "");
        view.setScanResults(scan);
    }

    void onUpdate() override {
        view.update();
    }

    void onExit() override {
        if (activeNetworkPreview == this)
            activeNetworkPreview = nullptr;
        view.destroy();
    }

    View& getView() override {
        return view;
    }

    bool handleUIEvent(const UIEvent& event) override {
        if (event.getType() == ButtonClickedEvent::TYPE &&
            static_cast<const ButtonClickedEvent&>(event).buttonId == "scan_networks")
            view.setScanResults(scan);
        return true;  // Preview never writes credentials or connects a radio.
    }

    void printDraft() {
        Serial.printf("@WIFI-DRAFT ssid=%s secure=%u password_bytes=%u\n", view.ssid().c_str(),
                      view.requiresPassword(), unsigned(view.secret().size()));
    }
};

class CatalogPreview : public Scene {
    GameListView view;
    GameInfo game;

  public:
    explicit CatalogPreview(const GameInfo& value) : game(value) {
    }

    void onEnter() override {
        view.setCatalog({game}, "", false);
        view.create();
    }

    void onUpdate() override {
        view.update();
    }

    void onExit() override {
        view.destroy();
    }

    View& getView() override {
        return view;
    }
};

// Display-only states: no card removal, Wi-Fi disconnect or fake backend health.
class IndicatorPreview : public Scene {
    MainMenuView view;
    unsigned mode;

  public:
    explicit IndicatorPreview(unsigned value) : mode(value) {
    }

    void onEnter() override {
        view.create();
        view.setStorageState(mode == 0   ? MainMenuView::StorageState::Ready
                             : mode == 1 ? MainMenuView::StorageState::Degraded
                             : mode == 2 ? MainMenuView::StorageState::Error
                                         : MainMenuView::StorageState::Unavailable);
    }

    void onUpdate() override {
        view.update();
    }

    void onExit() override {
        view.destroy();
    }

    View& getView() override {
        return view;
    }
};

// Preview game views without executing Lua, touching relays or starting a game.
class GamePreview : public Scene {
    GameView view;
    unsigned mode;

  public:
    explicit GamePreview(unsigned value) : mode(value) {
    }

    void onEnter() override {
        view.create();
        view.setTime(3600);
        if (mode == 1) {
            VictorySubview::GameStats stats;
            stats.elapsedSeconds = 1800;
            stats.pausedSeconds = 60;
            stats.hintsUsed = 2;
            stats.puzzlesSolved = 6;
            view.setVictoryStats(stats);
            view.showVictory();
        } else if (mode == 2)
            view.showFailure();
        else if (mode == 3) {
            GameObjectSubview::ObjectData object;
            object.title = "La mappa del laboratorio";
            object.content.push_back(GameObjectSubview::ContentBlock::textBlock(
                "Esamina gli indizi per trovare la prossima combinazione."));
            object.content.push_back(GameObjectSubview::ContentBlock::imageBlock(
                "/games/5-t_gameobject/images/tavola.png"));
            view.setGameObjectData({object});
            view.showGameObject();
        } else {
            view.showMain();
            view.setRunning(false);
            if (mode == 4)
                view.showPlayers(1);
        }
    }

    void onUpdate() override {
        view.update();
    }

    void onExit() override {
        view.destroy();
    }

    View& getView() override {
        return view;
    }
};

lv_obj_t* buttonAt(lv_obj_t* obj, int x, int y) {
    lv_point_t point{x, y};
    auto* hit = lv_indev_search_obj(obj, &point);
    while (hit) {
        if (lv_obj_check_type(hit, &lv_button_class))
            return hit;
        hit = lv_obj_get_parent(hit);
    }
    return nullptr;
}

void buttonCaption(lv_obj_t* obj, String& caption) {
    if (!lv_obj_is_visible(obj) || lv_obj_check_type(obj, &lv_textarea_class))
        return;
    if (lv_obj_check_type(obj, &lv_label_class)) {
        caption += lv_label_get_text(obj);
        caption += ' ';
    } else
        for (uint32_t i = 0; i < lv_obj_get_child_count(obj); ++i)
            buttonCaption(lv_obj_get_child(obj, i), caption);
}

lv_obj_t* scrollAt(lv_obj_t* obj, int x, int y) {
    if (!lv_obj_is_visible(obj))
        return nullptr;
    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    if (x < a.x1 || x > a.x2 || y < a.y1 || y > a.y2)
        return nullptr;
    for (int i = int(lv_obj_get_child_count(obj)) - 1; i >= 0; --i)
        if (auto* found = scrollAt(lv_obj_get_child(obj, i), x, y))
            return found;
    return lv_obj_has_flag(obj, LV_OBJ_FLAG_SCROLLABLE) ? obj : nullptr;
}

lv_obj_t* controlAt(lv_obj_t* obj, int x, int y) {
    if (!lv_obj_is_visible(obj))
        return nullptr;
    lv_area_t area;
    lv_obj_get_coords(obj, &area);
    if (x < area.x1 || x > area.x2 || y < area.y1 || y > area.y2)
        return nullptr;
    for (int i = int(lv_obj_get_child_count(obj)) - 1; i >= 0; --i)
        if (auto* found = controlAt(lv_obj_get_child(obj, i), x, y))
            return found;
    return lv_obj_check_type(obj, &lv_switch_class) || lv_obj_check_type(obj, &lv_slider_class)
               ? obj
               : nullptr;
}

void tree(lv_obj_t* obj) {
    if (!lv_obj_is_visible(obj))
        return;
    lv_area_t a;
    lv_obj_get_coords(obj, &a);
    if (lv_obj_check_type(obj, &lv_textarea_class))
        Serial.printf("@TEXTAREA %d %d %d %d masked=%d bytes=%u\n", a.x1, a.y1, a.x2, a.y2,
                      lv_textarea_get_password_mode(obj),
                      unsigned(strlen(lv_textarea_get_text(obj))));
    if (lv_obj_check_type(obj, &lv_switch_class))
        Serial.printf("@SWITCH %d %d %d %d value=%d\n", a.x1, a.y1, a.x2, a.y2,
                      lv_obj_has_state(obj, LV_STATE_CHECKED));
    if (lv_obj_check_type(obj, &lv_slider_class))
        Serial.printf("@SLIDER %d %d %d %d value=%d\n", a.x1, a.y1, a.x2, a.y2,
                      int(lv_slider_get_value(obj)));
    if (lv_obj_check_type(obj, &lv_button_class) &&
        buttonAt(lv_screen_active(), (a.x1 + a.x2) / 2, (a.y1 + a.y2) / 2) == obj) {
        char prefix[96];
        snprintf(prefix, sizeof(prefix), "@BUTTON %d %d %d %d disabled=%d ", a.x1, a.y1, a.x2, a.y2,
                 lv_obj_has_state(obj, LV_STATE_DISABLED));
        String row(prefix);
        for (uint32_t i = 0; i < lv_obj_get_child_count(obj); ++i) {
            auto* child = lv_obj_get_child(obj, i);
            buttonCaption(child, row);
        }
        Serial.println(row);
    }
    if (lv_obj_check_type(obj, &lv_label_class)) {
        bool inputLabel = false;
        for (auto* parent = lv_obj_get_parent(obj); parent; parent = lv_obj_get_parent(parent))
            if (lv_obj_check_type(parent, &lv_textarea_class)) {
                inputLabel = true;
                break;
            }
        // Password visibility is a user action; serial inspection still redacts it.
        String text = inputLabel ? "<input>" : lv_label_get_text(obj);
        text.replace("\n", " / ");
        Serial.printf("@LABEL %d %d %d %d %s\n", a.x1, a.y1, a.x2, a.y2, text.c_str());
    }
    for (uint32_t i = 0; i < lv_obj_get_child_count(obj); ++i)
        tree(lv_obj_get_child(obj, i));
}

void snapshot() {
    auto* buffer = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_RGB565);
    if (!buffer) {
        Serial.println("@SNAPSHOT ERROR allocation");
        return;
    }
    const unsigned width = buffer->header.w, height = buffer->header.h;
    // The actual display composites the top layer after the active screen.
    // Capture it separately with alpha so serial screenshots match the TFT.
    auto* overlay = lv_snapshot_take(lv_layer_top(), LV_COLOR_FORMAT_ARGB8888);
    if (!overlay) {
        lv_draw_buf_destroy(buffer);
        Serial.println("@SNAPSHOT ERROR overlay allocation");
        return;
    }
    for (unsigned y = 0; y < height && y < overlay->header.h; ++y) {
        auto* dst = buffer->data + y * buffer->header.stride;
        const auto* src =
            reinterpret_cast<const lv_color32_t*>(overlay->data + y * overlay->header.stride);
        for (unsigned x = 0; x < width && x < overlay->header.w; ++x) {
            const unsigned a = src[x].alpha;
            if (!a)
                continue;
            const unsigned color = dst[x * 2] | (unsigned(dst[x * 2 + 1]) << 8);
            const unsigned r =
                (unsigned(src[x].red) * a + ((color >> 11) & 31) * 255 / 31 * (255 - a) + 127) /
                255;
            const unsigned g =
                (unsigned(src[x].green) * a + ((color >> 5) & 63) * 255 / 63 * (255 - a) + 127) /
                255;
            const unsigned b =
                (unsigned(src[x].blue) * a + (color & 31) * 255 / 31 * (255 - a) + 127) / 255;
            const unsigned mixed = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            dst[x * 2] = uint8_t(mixed);
            dst[x * 2 + 1] = uint8_t(mixed >> 8);
        }
    }
    lv_draw_buf_destroy(overlay);
    Serial.printf("@SNAPSHOT BEGIN %u %u\n", width, height);
    char line[205];
    unsigned used = 4;
    std::memcpy(line, "@PX ", 4);
    const char hex[] = "0123456789abcdef";
    auto append = [&](uint8_t byte) {
        line[used++] = hex[byte >> 4];
        line[used++] = hex[byte & 15];
        if (used == 196) {
            line[used++] = '\n';
            Serial.write(reinterpret_cast<uint8_t*>(line), used);
            used = 4;
        }
    };
    auto pixel = [&](unsigned index) {
        const auto* p =
            buffer->data + (index / width) * buffer->header.stride + (index % width) * 2;
        return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
    };
    for (unsigned i = 0; i < width * height;) {
        const uint16_t color = pixel(i);
        unsigned count = 1;
        while (count < 255 && i + count < width * height && pixel(i + count) == color)
            ++count;
        append(count);
        append(color & 255);
        append(color >> 8);
        i += count;
    }
    if (used > 4) {
        line[used++] = '\n';
        Serial.write(reinterpret_cast<uint8_t*>(line), used);
    }
    Serial.println("@SNAPSHOT END");
    lv_draw_buf_destroy(buffer);
}

void checkStorage() {
    auto* storage = static_cast<StorageSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_STORAGE));
    if (!storage || !storage->isReady()) {
        Serial.println("@SDCHECK unavailable");
        return;
    }
    SPIBusLock::Guard guard;
    for (const char* path : {IMG_BRAND_NEGATIVE_FILE, IMG_LOGO_FILE})
        for (unsigned pass = 1; pass <= 2; ++pass) {
            auto file = storage->openNative(path, false);
            if (!file) {
                Serial.printf("@SDCHECK missing %s\n", path);
                continue;
            }
            const size_t expected = file.size();
            uint8_t buffer[512];
            uint32_t hash = 2166136261u;
            size_t total = 0;
            const uint32_t started = millis();
            while (total < expected) {
                const size_t count = file.read(buffer, min(sizeof(buffer), expected - total));
                if (!count || count > sizeof(buffer))
                    break;
                for (size_t i = 0; i < count; ++i)
                    hash = (hash ^ buffer[i]) * 16777619u;
                total += count;
            }
            const uint32_t elapsed = millis() - started;
            file.close();
            Serial.printf("@SDCHECK %s pass=%u bytes=%u/%u ms=%u fnv1a=%08lx\n", path, pass,
                          (unsigned)total, (unsigned)expected, (unsigned)elapsed,
                          (unsigned long)hash);
        }
}
}  // namespace

bool UIDiagnostics::touchSample(TouchPoint& sample) {
    if (simulatedTouch)
        sample = touchPoint;
    return simulatedTouch;
}

void UIDiagnostics::update(SceneManager& scenes) {
    static String command;
    while (Serial.available()) {
        const char value = Serial.read();
        if (value != '\n') {
            if (value != '\r' && command.length() < 96)
                command += value;
            continue;
        }
        if (command.startsWith("ui preview-language "))
            I18n::select(command.endsWith("en") ? I18n::Language::English
                                                : I18n::Language::Italian);
        else if (command.startsWith("ui wifi-preview ")) {
            WifiScanState state = command.endsWith("ready")      ? WifiScanState::Ready
                                  : command.endsWith("empty")    ? WifiScanState::Ready
                                  : command.endsWith("error")    ? WifiScanState::Error
                                  : command.endsWith("busy")     ? WifiScanState::Busy
                                  : command.endsWith("scanning") ? WifiScanState::Scanning
                                                                 : WifiScanState::Disabled;
            auto* preview = new NetworkPreview(state, command.endsWith("empty"));
            scenes.changeScene(preview, SceneTransition::None);
        } else if (command == "ui wifi-draft" && activeNetworkPreview)
            activeNetworkPreview->printDraft();
        else if (command == "ui wifi-status" || command.startsWith("ui wifi-fixture ") ||
                 command.startsWith("ui wifi-log "))
            networkConnectionDiagnostics(command.c_str());
        else if (command == "ui fixture clear")
            contentFixture.clear();
        else if (command.startsWith("ui fixture append ")) {
            const auto hex = command.substring(18);
            auto digit = [](char c) {
                return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : -1;
            };
            bool valid = !(hex.length() & 1) && contentFixture.size() + hex.length() / 2 <= 32768;
            for (unsigned i = 0; i < hex.length() && valid; ++i)
                valid = digit(hex[i]) >= 0;
            if (valid)
                for (unsigned i = 0; i < hex.length(); i += 2)
                    contentFixture += char(digit(hex[i]) * 16 + digit(hex[i + 1]));
            else
                Serial.println("@GAMECHECK FAIL invalid fixture chunk");
        } else if (command.startsWith("ui fixture lua ") ||
                   command.startsWith("ui fixture objects "))
            checkGameFixture(contentFixture, command.endsWith("en"),
                             command.startsWith("ui fixture objects "));
        else if (command.startsWith("ui fixture show ")) {
            JsonDocument document;
            if (deserializeJson(document, contentFixture) == DeserializationError::Ok) {
                GameInfo game{};
                game.id = "fixture";
                GameMetadata::read(document.as<JsonObjectConst>(), game);
                I18n::select(command.endsWith("en") ? I18n::Language::English
                                                    : I18n::Language::Italian);
                scenes.changeScene(new CatalogPreview(game), SceneTransition::None);
                Serial.printf("@GAMECHECK PASS metadata language=%s\n",
                              command.endsWith("en") ? "en" : "it");
            } else
                Serial.println("@GAMECHECK FAIL metadata JSON");
        } else if (command == "ui touch off")
            simulatedTouch = false;
        else if (command.startsWith("ui touch ")) {
            int x = 0, y = 0, down = 0;
            if (sscanf(command.c_str(), "ui touch %d %d %d", &x, &y, &down) == 3) {
                simulatedTouch = true;
                touchPoint = {x, y, down != 0};
            }
        } else if (command == "ui home")
            scenes.changeScene(new MainMenuScene(), SceneTransition::None);
        else if (command == "ui settings")
            scenes.changeScene(new SettingsScene(), SceneTransition::None);
        else if (command == "ui games")
            scenes.changeScene(new GameListScene(), SceneTransition::None);
        else if (command == "ui monitor")
            scenes.changeScene(new MonitorScene(), SceneTransition::None);
        else if (command == "ui about")
            scenes.changeScene(new AboutScene(), SceneTransition::None);
        else if (command == "ui network")
            scenes.changeScene(new NetworkScene(), SceneTransition::None);
        else if (command == "ui updates")
            scenes.changeScene(new UpdateScene(), SceneTransition::None);
        else if (command == "ui ota-test")
            runUpdateDiagnostics();
        else if (command == "ui hardware-test")
            runHardwareDiagnostics();
        else if (command.startsWith("ui hardware-game "))
            scenes.changeScene(createHardwareGameForDiagnostics(command.substring(17).c_str()), SceneTransition::None);
        else if (command == "ui hardware-state")
            hardwareStatusForDiagnostics();
        else if (command == "ui hardware-fault") {
            auto* input = static_cast<InputSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));
            if (input && GameScene::diagnosticCurrent) input->injectHardwareFaultForDiagnostics(HardwareIssueKind::MissingI2C);
        } else if (command.startsWith("ui room-id "))
            roomIdForDiagnostics(command.substring(11).toInt());
        else if (command.startsWith("ui ota-native ")) {
            unsigned bytes = 0;
            char digest[65]{};
            if (sscanf(command.c_str() + 14, "%u %64s", &bytes, digest) == 2)
                runNativeUpdateFlashDiagnostics(bytes, digest);
        } else if (command.startsWith("ui ota-preview"))
            scenes.changeScene(createUpdatePreview(command.endsWith("progress")     ? 1
                                                   : command.endsWith("error")      ? 2
                                                   : command.endsWith("installing") ? 3
                                                                                    : 0),
                               SceneTransition::None);
        else if (command == "ui ota-status") {
            const auto value = UpdateSystem::instance().snapshot();
            Serial.printf("@OTA state=%u error=%u downloaded=%u written=%u total=%u\n",
                          unsigned(value.state), unsigned(value.error), unsigned(value.downloaded),
                          unsigned(value.written), unsigned(value.total));
        } else if (command == "ui game")
            scenes.changeScene(new GamePreview(0), SceneTransition::None);
        else if (command == "ui players")
            scenes.changeScene(new GamePreview(4), SceneTransition::None);
        else if (command == "ui victory")
            scenes.changeScene(new GamePreview(1), SceneTransition::None);
        else if (command == "ui fail")
            scenes.changeScene(new GamePreview(2), SceneTransition::None);
        else if (command == "ui objects")
            scenes.changeScene(new GamePreview(3), SceneTransition::None);
        else if (command.startsWith("ui indicators ")) {
            unsigned mode = command.endsWith("ready")     ? 0
                            : command.endsWith("warning") ? 1
                            : command.endsWith("error")   ? 2
                                                          : 3;
            scenes.changeScene(new IndicatorPreview(mode), SceneTransition::None);
            scenes.previewIndicators(mode);
        } else if (command == "ui snapshot")
            snapshot();
        else if (command == "ui tree") {
            tree(lv_screen_active());
            tree(lv_layer_top());
        } else if (command == "ui perf reset")
            UIFrameStats::reset();
        else if (command == "ui perf")
            UIFrameStats::print();
        else if (command == "ui selftest")
            runSettingsDiagnostics();
        else if (command == "ui config-test")
            runConfigStorageDiagnostics();
        else if (command == "ui audio-test")
            runAudioDiagnostics();
        else if (command == "ui scroll-test")
            runScrollDiagnostics();
        else if (command == "ui players-test")
            runPlayerSelectorDiagnostics();
        else if (command == "ui volume-test")
            runVolumeFeedbackDiagnostics();
        else if (command == "ui preload-test")
            runAudioPreloadDiagnostics();
        else if (command == "ui assets-test")
            runGameAssetDiagnostics();
        else if (command == "ui assets-state") {
            const auto state = GameAssetCache::instance().status();
            Serial.printf(
                "@ASSETS files=%u bytes=%u hits=%u skipped=%u failed=%u resident=%u done=%u\n",
                unsigned(state.files), unsigned(state.bytes), unsigned(state.hits),
                unsigned(state.skipped), unsigned(state.failed),
                unsigned(GameAssetCache::residentBytes()), unsigned(state.done));
        } else if (command == "ui ingame-test")
            runInGameSettingsDiagnostics();
        else if (command == "ui stats-test")
            runStatisticsDiagnostics();
        else if (command == "ui statistics")
            scenes.changeScene(new StatisticsScene(), SceneTransition::None);
        else if (command == "ui stats-data")
            Serial.printf(
                "@STATS %s\n",
                StatisticsRepository::encode(StatisticsRepository::instance().records()).c_str());
        else if (command.startsWith("ui stats-sandbox "))
            Serial.printf("@STATSSANDBOX %s\n",
                          statisticsSandbox(command.endsWith("on")) ? "OK" : "BUSY");
        else if (command == "ui stats-win" && GameScene::diagnosticCurrent)
            GameScene::diagnosticCurrent->requestCompletion();
        else if (command == "ui preload-preview")
            scenes.changeScene(createAudioPreloadPreview(), SceneTransition::None);
        else if (command.startsWith("ui preload-event "))
            audioPreloadAction(command.substring(17).c_str());
        else if (command == "ui preferences") {
            DeviceConfig config;
            DeviceConfigRepository().load(config);
            Serial.printf("@PREFERENCES theme=%s language=%s music=%u/%u effects=%u/%u\n",
                          config.ui.theme.c_str(), config.locale.language.c_str(),
                          config.audio.music.enabled, config.audio.music.volume,
                          config.audio.effects.enabled, config.audio.effects.volume);
        } else if (command.startsWith("ui audio-master ")) {
            auto* audio = static_cast<AudioSystem*>(
                SystemEventBus::instance().getService(SystemEventBus::SERVICE_AUDIO));
            const int level = command.substring(16).toInt();
            if (audio && level >= 0 && level <= 100)
                audio->setVolume(level);
        } else if (command == "ui quiet on" || command == "ui quiet off") {
            DeviceConfig config;
            DeviceConfigRepository().load(config);
            const bool enabled = command.endsWith("off") && config.audio.effects.enabled &&
                                 config.audio.effects.volume;
            UIFeedback::setSoundEnabled(enabled);
            AudioEvent event{};
            event.type = AudioEventType::SET_VOLUME;
            event.volume.volume = command.endsWith("off") ? 100 : 0;
            xQueueSend(SystemEventBus::audioQueue, &event, 0);
        } else if (command == "ui sdcheck")
            checkStorage();
        else if (command.startsWith("ui theme "))
            UITheme::select(UITheme::fromName(command.substring(9).c_str()));
        else if (command.startsWith("ui click ")) {
            int x = 0, y = 0;
            if (sscanf(command.c_str(), "ui click %d %d", &x, &y) == 2)
                if (auto* button = buttonAt(lv_screen_active(), x, y))
                    if (!lv_obj_has_state(button, LV_STATE_DISABLED))
                        lv_obj_send_event(button, LV_EVENT_CLICKED, nullptr);
        } else if (command.startsWith("ui set ")) {
            int x = 0, y = 0, value = 0;
            if (sscanf(command.c_str(), "ui set %d %d %d", &x, &y, &value) == 3)
                if (auto* control = controlAt(lv_screen_active(), x, y))
                    if (!lv_obj_has_state(control, LV_STATE_DISABLED)) {
                        if (lv_obj_check_type(control, &lv_switch_class))
                            lv_obj_set_state(control, LV_STATE_CHECKED, value != 0);
                        else
                            lv_slider_set_value(control, value, LV_ANIM_OFF);
                        lv_obj_send_event(control, LV_EVENT_VALUE_CHANGED, nullptr);
                    }
        } else if (command.startsWith("ui scroll ")) {
            int y = 0;
            sscanf(command.c_str(), "ui scroll %d", &y);
            // SceneLayout's body is the second child. Settings panels scroll independently.
            if (auto* body = lv_obj_get_child(lv_screen_active(), 1))
                lv_obj_scroll_to_y(body, y, LV_ANIM_OFF);
        } else if (command.startsWith("ui scrollat ")) {
            int x = 0, y = 0, offset = 0;
            if (sscanf(command.c_str(), "ui scrollat %d %d %d", &x, &y, &offset) == 3)
                if (auto* target = scrollAt(lv_screen_active(), x, y))
                    lv_obj_scroll_to_y(target, offset, LV_ANIM_OFF);
        }
        Serial.printf("@UI DONE %s\n", command.c_str());
        command = "";
    }
}
#else
void UIDiagnostics::update(SceneManager&) {
}

bool UIDiagnostics::touchSample(TouchPoint&) {
    return false;
}
#endif
