# Device settings storage

Device settings use a single internal NVS record: namespace `console-cfg`, key
`settings`, JSON version 2. The repository has no SD dependency. A device without
a saved record uses firmware defaults.

| Data | Storage |
| --- | --- |
| Theme, language, music/effects switches and volumes | Internal NVS |
| Wi-Fi SSID and password | Internal NVS; SDK Wi-Fi persistence is disabled |
| Device name, brightness, timezone and date format | Internal NVS |
| Statistics and session recovery | Internal NVS `game-stats/snapshot` |
| OTA boot and rollback state | Internal NVS |
| Pins, hardware identity and compiled feature flags | Firmware build configuration |
| Lua games, game definitions, images, audio and portal assets | SD |
| Browser language and developer-mode preference | Browser localStorage |

## Settings format

The record contains `deviceName`, `wifi`, `locale`, `ui` and `audio`.
`audio.music` and `audio.effects` each contain `enabled` and `volume`.
Theme names are `mint`, `sky`, `coral`; language codes are `it-IT`, `en-GB`.

The record is bounded to 2048 bytes. Device name and timezone allow 64 UTF-8 bytes,
date format 32, brightness and volumes 0-100. SSIDs allow 32 bytes; passwords allow
an empty string for open networks, 8-63 bytes, or a 64-digit hexadecimal PSK.

All writes validate, commit and read back the record. Preference and network
updates merge only their fields under a shared lock. Identical saves avoid
rewrites. UI changes retain the 400 ms save debounce. Invalid records are reported;
partial saves do not overwrite them. Configuration reset writes firmware defaults.

Normal firmware/OTA updates preserve the NVS partition. Flash erasure and changes
to the partition layout can remove settings.

## Password handling

`/api/config` omits `wifi.password` in responses and reports
`wifi.passwordConfigured`. Omit the password on a PUT to keep it for the same SSID;
send an empty string explicitly for an open network. Changing SSID does not reuse
the previous password. Responses use `Cache-Control: no-store`.

Debug logging stays below Arduino verbose level because WebServer verbose logs
include request bodies. NVS is not encrypted in the current build; hardware-backed
encryption requires separate provisioning. The portal uses its existing local
HTTP transport.

## Verification

With debug diagnostics enabled:

```powershell
python scripts/diagnostics/ui_config_storage.py
```

The test writes only the isolated `cfg-check` NVS namespace, then verifies
preferences, configured network and statistics after reboot. Passwords are never
exported over serial. Real network workflows can be checked separately with
`ui_network_connection.py`, which temporarily backs up and restores settings.

```sh
cd frontend
npm run test:config
npm run build
```

Copy the rebuilt `sd/.core/portal` to the physical card when updating the portal.
Uploading firmware does not copy those assets.
