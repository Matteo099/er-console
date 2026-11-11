# EscapeRoom Console

EscapeRoom Console is an ESP32-S3 runtime for escape room games. It runs Lua games from a microSD card and provides a TFT touchscreen interface, hardware integrations, audio, and a Vue-based management portal.

## Getting started

Install PlatformIO and Node.js (see `.nvmrc`), then from the repository root:

```sh
pio run
cd frontend
npm ci
npm run build
```

Prepare the microSD card and upload the firmware as described in [Getting Started](wiki/Getting-Started.md). See the [wiki](wiki/Home.md) for configuration, development, releases, and troubleshooting.

## Repository

- `src/`, `include/`, `boards/`: firmware, configuration, and board definition.
- `frontend/`: web portal source.
- `sd/`: runtime audio, images, and Lua games.
- `scripts/`: build tools, diagnostics, and tests.
- `wiki/`: project documentation.
