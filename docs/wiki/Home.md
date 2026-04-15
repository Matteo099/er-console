# EscapeRoom Console Wiki

EscapeRoom Console is an ESP32-S3 based runtime for Escape Room experiences, with game logic in Lua and a C++ firmware layer for hardware, UI, networking, and storage.

## What This Project Provides

- Firmware OS for ESP32-S3 (Arduino framework)
- Scene-driven device UI (LVGL + LovyanGFX)
- Lua runtime for dynamic game content from SD card
- Captive portal and device management API
- Vue 3 + Vuetify admin frontend served by the device

## Quick Links

- [[Architecture]]
- [[Project-Structure]]
- [[UI-TFT-Scenes]]
- [[Firmware-Setup]]
- [[Frontend-Portal]]
- [[REST-API]]
- [[Contributing]]
- [[Troubleshooting]]

## High-Level Stack

1. Lua game scripts (scenario logic, puzzles, flow)
2. Lua bindings / API layer
3. EscapeRoom OS in C++ (boot, scenes, systems)
4. ESP32 Arduino framework
5. ESP32-S3 hardware

## Runtime Storage Layout (SD)

- `/.core/config/config.json` for device configuration
- `/.core/portal` for web portal static assets
- `/games` for Lua game packages

## Captive Portal Summary

- Access point starts from firmware with SSID based on device name
- DNS wildcard is enabled for captive onboarding
- Default portal URL falls back to http://192.168.4.1/
- Frontend SPA communicates with REST endpoints under `/api/*`

## Repository

Main repository structure combines firmware and frontend in one workspace:

- Firmware and embedded systems code in `src/`
- Frontend portal app in `frontend/`
- Build and board configuration in `platformio.ini` and `boards/`
