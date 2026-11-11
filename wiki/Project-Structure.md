# Project Structure

## Top-Level

- `platformio.ini`: PlatformIO environment and build flags
- `boards/`: custom board definition
- `include/`: firmware include-time config (LVGL, TFT, etc.)
- `src/`: firmware C++ source
- `sd/`: sample SD card content (games and assets)
- `frontend/`: web portal SPA source
- `scripts/`: build/pre-build helper scripts
- `wiki/`: project documentation

## Firmware Source Layout (`src/`)

- `config/`: path and asset constants
- `core/`: system event bus and core contracts
- `drivers/`: low-level display/audio/storage and hardware interfaces
- `log/`: logger and in-memory log retrieval/cleanup
- `os/`: boot manager and OS entry behavior
- `scenes/`: UI scene modules (boot, menu, game, network, settings, etc.)
- `services/`: repositories and higher-level data/service abstractions
- `systems/`: subsystem implementations (audio, input, lua, network, portal, storage, ui)

## Frontend Source Layout (`frontend/src/`)

- `components/`: reusable Vue components
- `models/`: TypeScript interfaces for API and UI data
- `plugins/`: i18n and Vuetify plugin configuration
- `router/`: page routing
- `services/`: API wrappers and optional mock backend
- `stores/`: Pinia stores for app state
- `views/`: page-level UI screens

## Important Runtime Paths

- `/.core/portal/index.html`: frontend entry served by firmware
- `/.core/config/config.json`: device configuration file
- `/games/*`: Lua game payloads

## Scripts

- `scripts/platformio/`: pre-build hooks selected in `platformio.ini`, plus the inactive TFT_eSPI setup helper `update_user_setup.py`.
- `scripts/ci/`: release packaging and CI checks.
- `scripts/checks/`: translation validation used in CI.
- `scripts/diagnostics/`: serial capture and hardware regression tools; these require a device and are not CI unit tests.
- `scripts/audio/`: audio generation and conversion utilities; `scripts/tests/` contains the conversion unit tests.
- `scripts/fonts/`: font generators and pinned LVGL conversion dependencies.
- `scripts/extension/`: Lua API stub generator used by the editor.
