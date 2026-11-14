#include "config/Console.h"
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
#include "HardwareDiagnostics.h"
#include "systems/input/HardwareCheck.h"
#include "systems/input/InputSystem.h"
#include "drivers/input/RoomHardwareProbe.h"
#include "scenes/game/GameScene.h"
#include "core/SystemEventBus.h"
#include "systems/ui/UIFeedback.h"
#include "systems/ui/UIEvents.h"
#include <OneWire.h>
#include <algorithm>
#include <cstring>

namespace {
std::string rom(unsigned serial) {
    uint8_t bytes[8]{1, uint8_t(serial), 0, 0, 0, 0, 0, 0};
    bytes[7] = OneWire::crc8(bytes, 7);
    char out[17]{};
    for (unsigned i = 0; i < 8; ++i)
        snprintf(out + 2 * i, 3, "%02X", bytes[i]);
    return out;
}

struct Probe : HardwareProbe {
    bool present = true, reader = true;
    unsigned i2cCalls = 0, rfidCalls = 0, identityCalls = 0;
    std::string room = rom(1);

    bool i2c(uint8_t) override {
        ++i2cCalls;
        return present;
    }

    bool rfid(uint8_t, uint8_t, bool) override {
        ++rfidCalls;
        return reader;
    }

    std::string identity(uint8_t) override {
        ++identityCalls;
        return room;
    }
};

InputConfig parse(const char* text) {
    JsonDocument json;
    deserializeJson(json, text);
    return InputConfig::parse(json.as<JsonVariantConst>());
}

InputSystem* input() {
    return static_cast<InputSystem*>(
        SystemEventBus::instance().getService(SystemEventBus::SERVICE_INPUT));
}

lv_obj_t* button(lv_obj_t* root, const char* caption) {
    if (!lv_obj_is_visible(root))
        return nullptr;
    if (lv_obj_check_type(root, &lv_label_class) &&
        !std::strcmp(lv_label_get_text(root), caption) &&
        lv_obj_check_type(lv_obj_get_parent(root), &lv_button_class))
        return lv_obj_get_parent(root);
    for (unsigned i = 0; i < lv_obj_get_child_count(root); ++i)
        if (auto* found = button(lv_obj_get_child(root, i), caption))
            return found;
    return nullptr;
}

struct Capture : UIEventHandler {
    unsigned retries = 0, backs = 0;

    bool handleUIEvent(const UIEvent& event) override {
        if (event.getType() != ButtonClickedEvent::TYPE)
            return false;
        const auto& name = static_cast<const ButtonClickedEvent&>(event).buttonId;
        if (name == "retry")
            ++retries;
        if (name == "back")
            ++backs;
        return true;
    }
};
}  // namespace

void runHardwareDiagnostics() {
    unsigned passed = 0, failed = 0;
    const auto check = [&](bool okay, const char* name) {
        okay ? ++passed : ++failed;
        Serial.printf("@HWCHECK %s %s\n", okay ? "PASS" : "FAIL", name);
    };
    Probe probe;
    InputConfig config;
    check(HardwareCheck::inspect(config, probe, true).empty() && !probe.i2cCalls &&
              !probe.rfidCalls && !probe.identityCalls,
          "software-only games require no room hardware");
    config = parse(
        R"({"power":{"enabled":true},"oled":{"enabled":true},"channels":{"enabled":true,"mcp1Enabled":true,"mcp2Enabled":true},"rgb":{"enabled":true,"usePCA9685":true,"address":65},"rfid":{"enabled":true}})");
    check(HardwareCheck::inspect(config, probe, true).empty() && probe.i2cCalls == 5 &&
              probe.rfidCalls == 1,
          "all declared readable devices must respond");
    probe.present = false;
    probe.rfidCalls = 0;
    check(HardwareCheck::inspect(config, probe, true).size() == 5 && !probe.rfidCalls,
          "missing I2C profile reports devices without touching RFID output pins");
    config = InputConfig{};
    config.rfid.enabled = true;
    probe.reader = false;
    const auto rfid = HardwareCheck::inspect(config, probe, false);
    check(rfid.size() == 1 && rfid[0].kind == HardwareIssueKind::MissingRFID,
          "absent RFID is a hardware failure, not an absent card");
    config = InputConfig{};
    config.connection.enabled = true;
    config.connection.rom = rom(1);
    probe.room = rom(1);
    check(HardwareCheck::validRoomId(rom(1)) &&
              HardwareCheck::inspect(config, probe, false).empty(),
          "DS2401 identity accepts the expected family and CRC-verified ROM");
    probe.room = rom(2);
    probe.i2cCalls = probe.rfidCalls = 0;
    auto issues = HardwareCheck::inspect(config, probe, true);
    check(issues.size() == 1 && issues[0].kind == HardwareIssueKind::WrongRoom && !probe.i2cCalls &&
              !probe.rfidCalls,
          "different room identity blocks other hardware initialization");
    probe.room.clear();
    issues = HardwareCheck::inspect(config, probe, false);
    check(issues.size() == 1 && issues[0].kind == HardwareIssueKind::MissingIdentity,
          "missing room identifier detects an unplugged cable");
    probe.room = rom(1);
    probe.room.back() = probe.room.back() == '0' ? '1' : '0';
    issues = HardwareCheck::inspect(config, probe, false);
    check(issues.size() == 1 && issues[0].kind == HardwareIssueKind::MissingIdentity,
          "corrupt 1-Wire data cannot authorize a room");
    for (const char* text :
         {R"({"power":{"enabled":"true"}})", R"({"power":{"enabled":true,"address":300}})",
          R"({"power":{"enabled":true,"address":-1}})", R"({"rfid":{"enabled":true,"ssPin":10}})",
          R"({"power":{"enabled":true},"rgb":{"enabled":true,"usePCA9685":true}})",
          R"({"channels":{"enabled":true,"mcp1Enabled":true,"mcp2Enabled":true,"mcp2Base":110}})",
          R"({"relays":{"enabled":true}})",
          R"({"sevenSegment":{"enabled":true,"csPin":39,"digits":9}})",
          R"({"connection":{"enabled":true,"pin":48,"rom":"bad"}})", "[]"}) {
        config = parse(text);
        probe.i2cCalls = probe.rfidCalls = probe.identityCalls = 0;
        check(!HardwareCheck::inspect(config, probe, true).empty() && !probe.i2cCalls &&
                  !probe.rfidCalls && !probe.identityCalls,
              "invalid configuration is rejected before any bus or GPIO operation");
    }
    for (const char* text :
         {R"({"adc":{"enabled":true}})", R"({"channels":{"enabled":true,"mcp3Enabled":true}})",
          R"({"i2c":{"sdaPin":1,"sclPin":2}})", R"({"rfid":{"enabled":true,"spiBus":"tft"}})"}) {
        issues = parse(text).validate();
        check(std::any_of(issues.begin(), issues.end(),
                          [](const HardwareIssue& issue) {
                              return issue.kind == HardwareIssueKind::Unsupported;
                          }),
              "unsupported requested hardware is explicitly rejected");
    }
    check(parse(R"({"adc":{"enabled":false}})").validate().empty(),
          "disabled optional hardware is not required");
    HardwareCheck::Monitor monitor;
    check(!monitor.sample(false) && !monitor.sample(true) && !monitor.sample(false) &&
              monitor.sample(false),
          "consecutive failures stop monitoring while isolated glitches are filtered");
    NativeChannelProvider channels({38, 41});
    check(!channels.owns(38) && !channels.owns(41) && !channels.owns(10) && channels.owns(42),
          "game GPIO access cannot claim room identity, driver or console pins");
    const bool sound = UIFeedback::soundEnabled();
    UIFeedback::setSoundEnabled(false);
    auto* original = lv_display_get_default();
    auto* display = lv_display_create(320, 240);
    lv_display_set_default(display);
    {
        GameView view;
        Capture events;
        view.setEventHandler(&events);
        view.create();
        auto* screen = view.getScreen();
        lv_screen_load(screen);
        HardwareStatus status;
        status.state = HardwareState::Fault;
        status.issues = {{HardwareIssueKind::MissingI2C, "MCP23017 #1 (I2C 0x20)"},
                         {HardwareIssueKind::MissingIdentity, "DS2401"}};
        view.showHardwareError(status, false);
        lv_obj_update_layout(screen);
        auto* retry = button(screen, tr(Text::Retry));
        auto* back = button(screen, tr(Text::Back));
        lv_area_t area{};
        if (retry)
            lv_obj_get_coords(retry, &area);
        check(view.currentSubview() == GameSubview::Hardware && retry && back && area.y1 >= 192 &&
                  area.y2 < 240 && area.x1 >= 0 && area.x2 < 320,
              "hardware errors offer fixed visible retry/back controls without failure music");
        if (retry)
            lv_obj_send_event(retry, LV_EVENT_CLICKED, nullptr);
        if (back)
            lv_obj_send_event(back, LV_EVENT_CLICKED, nullptr);
        check(events.retries == 1 && events.backs == 1,
              "hardware error actions use existing scene events");
        view.showHardwareError(status, true);
        check(view.getScreen() == screen && !view.settingsVisible() && !view.playersVisible(),
              "connection loss dismisses game overlays and keeps the same scene");
        view.destroy();
    }
    lv_display_delete(display);
    lv_display_set_default(original);
    UIFeedback::setSoundEnabled(sound);
    Serial.printf("@HWTEST passed=%u failed=%u stack=%u\n", passed, failed,
                  unsigned(uxTaskGetStackHighWaterMark(nullptr)));
}

Scene* createHardwareGameForDiagnostics(const char* mode) {
    GameInfo info{};
    info.id = "3-t_loop";
    info.title = "Hardware test";
    info.durationMin = 1;
    InputConfig config;
    if (!std::strcmp(mode, "missing")) {
        config.channels.enabled = true;
        config.channels.mcp1Enabled = true;
        config.channels.mcp1Address = 0x27;
    } else if (!std::strcmp(mode, "identity")) {
        config.connection.enabled = true;
        config.connection.rom = rom(1);
    } else if (!std::strcmp(mode, "invalid")) {
        config.rfid.enabled = true;
        config.rfid.ssPin = TFT_CS_PIN;
    }
    auto* scene = new GameScene();
    scene->setGameInfo(info);
    scene->useHardwareForDiagnostics(config);
    return scene;
}

void hardwareStatusForDiagnostics() {
    if (!input())
        return;
    const auto status = input()->hardwareStatus();
    auto* lua =
        static_cast<LuaSystem*>(SystemEventBus::instance().getService(SystemEventBus::SERVICE_LUA));
    Serial.printf("@HWSTATE state=%u issues=%u lua=%u\n", unsigned(status.state),
                  unsigned(status.issues.size()), lua && lua->isScriptLoaded());
}

void roomIdForDiagnostics(unsigned pin) {
    if (pin > 255 || !HardwareCheck::roomPin(pin) || GameScene::diagnosticCurrent) {
        Serial.println("@ROOMID invalid-pin-or-game-active");
        return;
    }
    RoomHardwareProbe probe;
    const auto id = probe.identity(pin);
    pinMode(pin, INPUT);
    Serial.printf("@ROOMID pin=%u rom=%s\n", pin, id.empty() ? "not-detected" : id.c_str());
}
#endif
