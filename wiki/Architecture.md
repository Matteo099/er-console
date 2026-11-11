# Architecture

## Design Principles

- Keep game logic outside firmware (Lua from SD card)
- Keep hardware access inside C++ service and driver layers
- Keep UI and system concerns isolated by scene/system boundaries
- Support runtime scenario updates without firmware rebuild

## Layered View

```text
+-------------------------------------------+
|              LUA SCRIPTS                  |
|      scenarios, puzzles, state flow       |
+-------------------------------------------+
|             LUA BINDINGS API              |
|     display, input, audio, storage        |
+-------------------------------------------+
|             ESCAPEROOM OS (C++)           |
| boot | scenes | systems | services | log  |
+-------------------------------------------+
|          ESP32 ARDUINO FRAMEWORK          |
+-------------------------------------------+
|              ESP32-S3 HARDWARE            |
+-------------------------------------------+
```

## Main Runtime Components

- Boot and OS orchestration: `src/os/`
- Scene flow and UX: `src/scenes/`
- Hardware and infra systems: `src/systems/`
- Device services and repositories: `src/services/`
- Drivers: `src/drivers/`
- Logging: `src/log/`

## Portal Architecture

- `PortalSystem` starts AP mode, DNS, web server, and static file serving
- Controllers split by concern:
  - `StatusController`: status, wifi scan, logs, reboot/shutdown
  - `ConfigController`: config read/update/reset
  - `FilesController`: list, delete, move, upload
- Static portal assets are served from `/.core/portal`

## Data Persistence

- Device config is stored in `/.core/config/config.json`
- Config defaults are defined in `DeviceConfig`
- Storage and network systems are injected into portal service layer

## Frontend Integration

- Vue 3 + Vite + TypeScript frontend in `frontend/`
- Vuetify UI with i18n locales
- API client under `frontend/src/services/`
- Optional local mock server via `VITE_LOCAL_SERVER=true`
