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
#include <algorithm>
#include <cstring>

namespace {
struct Probe : HardwareProbe {
    bool present = true, reader = true;
    unsigned i2cCalls = 0, rfidCalls = 0, identityCalls = 0;
    RoomIdentityReading room;

    bool i2c(uint8_t) override {
        ++i2cCalls;
        return present;
    }

    bool rfid(uint8_t, uint8_t, bool) override {
        ++rfidCalls;
        return reader;
    }

    RoomIdentityReading identity(uint8_t) override {
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
    config.connection.resistanceOhms = 220;
    probe.room.millivolts = HardwareCheck::identityVoltage(220);
    check(HardwareCheck::inspect(config, probe, false).empty(),
          "220 ohm room and identical clones authorize the same game");
    config.power.enabled = true;
    config.rfid.enabled = true;
    probe.room.millivolts = HardwareCheck::identityVoltage(380);
    probe.i2cCalls = probe.rfidCalls = 0;
    auto issues = HardwareCheck::inspect(config, probe, true);
    check(issues.size() == 1 && issues[0].kind == HardwareIssueKind::WrongRoom && !probe.i2cCalls &&
              !probe.rfidCalls,
          "different room identity blocks other hardware initialization");
    config.power.enabled = false;
    config.rfid.enabled = false;
    probe.room.millivolts = 3100;
    issues = HardwareCheck::inspect(config, probe, false);
    check(issues.size() == 1 && issues[0].kind == HardwareIssueKind::MissingIdentity,
          "missing room identifier detects an unplugged cable");
    probe.room.millivolts = 0;
    issues = HardwareCheck::inspect(config, probe, false);
    check(issues.size() == 1 && issues[0].kind == HardwareIssueKind::IdentityShort,
          "shorted identification wire cannot authorize a room");
    probe.room.millivolts = HardwareCheck::identityVoltage(220);
    probe.room.spreadMillivolts = HardwareCheck::IdentityMaxSpreadMillivolts + 1;
    issues = HardwareCheck::inspect(config, probe, false);
    check(issues.size() == 1 && issues[0].kind == HardwareIssueKind::IdentityUnstable,
          "noisy contacts cannot authorize a room");
    probe.room.spreadMillivolts = 0;
    for (unsigned resistance : {150u, 220u, 380u, 390u, 680u, 1000u, 2200u, 4700u}) {
        config.connection.resistanceOhms = resistance;
        const unsigned low = HardwareCheck::identityVoltage(resistance * 90 / 100) - 25;
        const unsigned high = HardwareCheck::identityVoltage(resistance * 110 / 100) + 25;
        probe.room.millivolts = low;
        const bool acceptsLow = HardwareCheck::inspect(config, probe, false).empty();
        probe.room.millivolts = high;
        const bool acceptsHigh = HardwareCheck::inspect(config, probe, false).empty();
        probe.room.millivolts = low - 1;
        const bool rejectsLow = !HardwareCheck::inspect(config, probe, false).empty();
        probe.room.millivolts = high + 1;
        const bool rejectsHigh = !HardwareCheck::inspect(config, probe, false).empty();
        check(acceptsLow && acceptsHigh && rejectsLow && rejectsHigh,
              "resistor windows include tolerance and ADC margin with strict boundaries");
    }
    config.connection.resistanceOhms = 380;
    probe.room.millivolts = HardwareCheck::identityVoltage(220);
    check(!HardwareCheck::inspect(config, probe, false).empty(),
          "220 and 380 ohm games cannot authorize one another");
    config.connection.resistanceOhms = 220;
    config.connection.pin = 2;
    check(config.validate().empty(), "the second free ADC1 pin is supported");
    check(
        parse(R"({"connection":{"enabled":true,"pin":1,"resistanceOhms":220}})").validate().empty(),
        "the documented resistor configuration parses successfully");
    for (
        const char* text :
        {R"({"power":{"enabled":"true"}})", R"({"power":{"enabled":true,"address":300}})",
         R"({"power":{"enabled":true,"address":-1}})", R"({"rfid":{"enabled":true,"ssPin":10}})",
         R"({"power":{"enabled":true},"rgb":{"enabled":true,"usePCA9685":true}})",
         R"({"channels":{"enabled":true,"mcp1Enabled":true,"mcp2Enabled":true,"mcp2Base":110}})",
         R"({"relays":{"enabled":true}})",
         R"({"sevenSegment":{"enabled":true,"csPin":39,"digits":9}})",
         R"({"connection":{"enabled":true,"pin":48,"resistanceOhms":220}})",
         R"({"connection":{"enabled":true,"pin":38,"resistanceOhms":220}})",
         R"({"connection":{"enabled":true}})",
         R"({"connection":{"enabled":true,"resistanceOhms":149}})",
         R"({"connection":{"enabled":true,"resistanceOhms":4701}})",
         R"({"connection":{"enabled":true,"resistanceOhms":-1}})",
         R"({"connection":{"enabled":true,"resistanceOhms":"220"}})",
         R"({"connection":{"enabled":true,"resistanceOhms":220.5}})",
         R"({"connection":{"enabled":true,"resistanceOhms":4294967296}})",
         R"({"connection":{"enabled":true,"resistanceOhms":220},"rfid":{"enabled":true,"ssPin":1}})",
         "[]"}) {
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
    NativeChannelProvider channels({1, 41});
    check(!channels.owns(1) && !channels.owns(41) && !channels.owns(10) && channels.owns(42),
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
                         {HardwareIssueKind::MissingIdentity, "GPIO 1"}};
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
        config.connection.resistanceOhms = 220;
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

void roomResistorForDiagnostics(unsigned pin) {
    if (pin > 255 || !HardwareCheck::identityPin(pin) || GameScene::diagnosticCurrent) {
        Serial.println("@ROOMRESISTOR invalid-pin-or-game-active");
        return;
    }
    RoomHardwareProbe probe;
    const auto id = probe.identity(pin);
    pinMode(pin, INPUT);
    const char* state = id.spreadMillivolts > HardwareCheck::IdentityMaxSpreadMillivolts
                            ? "unstable"
                        : id.millivolts >= HardwareCheck::IdentityOpenMillivolts  ? "open"
                        : id.millivolts <= HardwareCheck::IdentityShortMillivolts ? "short"
                                                                                  : "measured";
    const unsigned ohms = id.millivolts < HardwareCheck::IdentitySupplyMillivolts
                              ? uint64_t(HardwareCheck::IdentityPullupOhms) * id.millivolts /
                                    (HardwareCheck::IdentitySupplyMillivolts - id.millivolts)
                              : 0;
    Serial.printf("@ROOMRESISTOR pin=%u mv=%u spread=%u ohms=%u state=%s\n", pin, id.millivolts,
                  id.spreadMillivolts, ohms, state);
}
#endif
