# Troubleshooting

## Portal Not Reachable

Symptoms:

- No portal page on phone/laptop
- Device AP visible but captive page does not open

Checks:

1. Confirm AP SSID is visible (`<deviceName> ER-WiFi`)
2. Open portal URL manually from status screen (usually http://192.168.4.1/)
3. Verify `/.core/portal/index.html` exists on SD card
4. Reboot device via hardware reset or `/api/system/reboot`

## Frontend Works Locally But Not On Device

Checks:

1. Run `npm run build` in `frontend/`
2. Copy built assets to `/.core/portal`
3. Ensure compressed files are copied if your deploy process uses gzip
4. Confirm storage is mounted and readable at boot

## Config Is Not Persisting

Checks:

1. Check for NVS configuration/write errors; device settings do not require SD
2. If migration is pending, retain the original SD files and see [storage and migration](reference/DEVICE_SETTINGS_STORAGE.md); do not erase flash to fix a save error
3. Validate JSON payload shape for `PUT /api/config`
4. Check logs from `GET /api/system/logs`

## File Upload Fails

Checks:

1. Ensure request is multipart with field name `file`
2. Include `path` query parameter (for example `/.core/upload.bin`)
3. Verify destination directory exists and storage has free space
4. Check for storage errors in system logs

## Build Issues

Firmware:

- Re-run PlatformIO dependency install
- Verify board environment name in `platformio.ini`
- Ensure pre-build scripts in `scripts/` execute successfully

Frontend:

- Remove `node_modules` and reinstall
- Confirm Node.js version is compatible with current Vite/Vue setup
- Re-run lint and build to capture type/runtime build errors
