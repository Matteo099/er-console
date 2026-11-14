#pragma once

#include <vector>
#include "config/Console.h"

#include "core/SystemService.h"
#include "core/TaskMutex.h"

#include "subsystems/ISubsystem.h"
#include "InputConfig.h"

#include "drivers/input/power/INA219Driver.h"
#include "drivers/input/rfid/MFRC522Reader.h"
#include "HardwareCheck.h"
#include "drivers/input/oled/SSD1306Driver.h"
#include "drivers/input/channel/NativeChannelProvider.h"
#include "drivers/input/channel/MCPChannelProvider.h"
#include "drivers/input/channel/MuxChannelProvider.h"
#include "drivers/input/rgb/ESP32PwmRgbDriver.h"
#include "drivers/input/rgb/PCA9685Driver.h"
#include "drivers/input/sevensegment/MAX7219Driver.h"
#include "drivers/bus/I2CBusManager.h"

#include "subsystems/PowerSystem.h"
#include "subsystems/RFIDSystem.h"
#include "subsystems/OLEDSystem.h"
#include "subsystems/ChannelSystem.h"
#include "subsystems/RelaySystem.h"
#include "subsystems/RgbLedSystem.h"
#include "subsystems/SevenSegmentSystem.h"

class InputSystem : public SystemService {
  public:
    InputSystem();
    ~InputSystem();

    void init(void* params = nullptr) override;
    void requestBegin(const InputConfig& config);
    void requestHardwareCheck();
    HardwareStatus hardwareStatus() const;
    bool powerSnapshot(PowerReading& reading);
#if UI_DIAGNOSTICS_ENABLED && defined(CORE_DEBUG_LEVEL) && CORE_DEBUG_LEVEL > 0
    void injectHardwareFaultForDiagnostics(HardwareIssueKind kind);
#endif
    void update() override;
    void shutdown();

    bool hasPower() const {
        return powerInitialized;
    }

    PowerSystem* power();
    RFIDSystem* rfid();
    OLEDSystem* oled();
    ChannelSystem* channels();
    RelaySystem* relays();
    RgbLedSystem* rgb();
    SevenSegmentSystem* sevenSegment();

  private:
    TaskMutex lifecycleMutex;
    mutable TaskMutex hardwareMutex;
    HardwareStatus hardware;
    InputConfig queuedConfig, activeConfig;
    bool beginPending = false, checkPending = false, monitoring = false,
         connectionConfigured = false;
    uint32_t generation = 0, checkedAt = 0;
    HardwareCheck::Monitor monitor;
    bool begin(const InputConfig& config, uint32_t token);
    void publish(uint32_t token, HardwareState state, std::vector<HardwareIssue> issues = {});
    bool powerInitialized = false;
    bool rfidInitialized = false;
    bool oledInitialized = false;
    bool channelInitialized = false;
    bool rgbInitialized = false;
    bool sevenSegmentInitialized = false;
    bool relayInitialized = false;

    I2CBusManager i2cManager;

    std::vector<ISubsystem*> subsystems;

    INA219Driver* powerDriver = nullptr;
    MFRC522Reader* rfidDriver = nullptr;
    SSD1306Driver* oledDriver = nullptr;
    NativeChannelProvider* nativeChannel = nullptr;
    MCP23017Driver* mcpDriver1 = nullptr;
    MCP23017Driver* mcpDriver2 = nullptr;
    CD74HC4067Driver* muxDriver = nullptr;
    MCPChannelProvider* mcp1 = nullptr;
    MCPChannelProvider* mcp2 = nullptr;
    MuxChannelProvider* mux1 = nullptr;
    ESP32PwmRgbDriver* statusLed = nullptr;
    PCA9685Driver* rgbExpander = nullptr;
    MAX7219Driver* sevenSegDriver = nullptr;

    PowerSystem* powerSystem = nullptr;
    RFIDSystem* rfidSystem = nullptr;
    OLEDSystem* oledSystem = nullptr;
    ChannelSystem* channelSystem = nullptr;
    RelaySystem* relaySystem = nullptr;
    RgbLedSystem* rgbLedSystem = nullptr;
    SevenSegmentSystem* sevenSegmentSystem = nullptr;

    void reset();
    void clearFlags();
    void destroyChannels();
    void destroyPower();
    void destroyRfid();
    void destroyOled();
    void destroyRgb();
    void destroySevenSegment();
    void destroyRelay();
};
