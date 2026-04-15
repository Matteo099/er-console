# Firmware Setup

## Requirements

- PlatformIO (VS Code extension or CLI)
- ESP32-S3 target board profile available
- SD card prepared for `/.core` and `/games` content

## PlatformIO Environment

The default environment is:

- `env:esp32-s3-devkitc-1-n16r8v`
- Framework: Arduino
- MCU: ESP32-S3
- Flash: 16MB
- PSRAM: OPI enabled

Key libraries include:

- lvgl
- LovyanGFX
- lua
- ArduinoJson
- Adafruit NeoPixel

## Build

From repository root:

```bash
pio run
```

## Upload

```bash
pio run -t upload
```

## Serial Monitor

```bash
pio device monitor -b 115200
```

## Notes on Build Scripts

`platformio.ini` runs pre-build scripts in `scripts/` to sync display configuration, adjust Lua setup, and stamp git metadata.

## Configuration Persistence

At runtime, configuration is loaded from and saved to:

- `/.core/config/config.json`

Default values include:

- Device name: EscapeRoom Console
- Locale: it-IT / Europe-Rome style defaults
- UI theme and brightness defaults
- Audio volume and enabled state

## Captive Portal Runtime

During portal startup firmware:

1. Enables AP+STA mode
2. Starts AP with generated SSID (`<deviceName> ER-WiFi`)
3. Starts DNS wildcard for captive behavior
4. Registers REST and static routes
5. Serves frontend files from `/.core/portal`
