# Getting Started

Install PlatformIO (CLI or VS Code extension), Python, and the Node.js version in `.nvmrc`. The supported board is ESP32-S3 N16R8.

1. Copy the contents of `sd/`, including the hidden `.core` directory, to the root of a microSD card.
2. From the repository root run `pio run` and `pio run -t upload`. The default environment is the debug build; select the release environment with `-e esp32-s3-devkitc-1-n16r8v`.
3. Build the portal with `npm ci` and `npm run build` in `frontend/`. The build copies its output to `sd/.core/portal`; copy that directory to the card.
4. Insert the card and use `pio device monitor -b 115200` for serial output.

The portal and Wi-Fi depend on firmware configuration. See [Firmware Setup](Firmware-Setup.md), [Frontend Portal](Frontend-Portal.md), and [Troubleshooting](Troubleshooting.md).
