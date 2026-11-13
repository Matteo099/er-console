# Device settings storage

Personal and device-specific configuration is stored in the ESP32's internal
NVS partition, namespace `console-cfg`, key `settings`. It is not a file on SD.
The version-2 JSON snapshot is bounded to 2048 bytes and validated on every read.

## Storage audit

| Data | Storage and purpose |
| --- | --- |
| TFT theme, language, music/effects switches and volumes | Internal NVS snapshot |
| Wi-Fi SSID and password, including explicitly cleared credentials | Internal NVS snapshot; Wi-Fi SDK persistence is disabled |
| Device name, brightness, timezone and date format | Internal NVS snapshot |
| Statistics and session recovery | Existing internal NVS `game-stats/snapshot` |
| OTA pending version, boot result and rollback bookkeeping | Existing internal NVS; retained by application updates |
| Pin assignments, board identity and compiled feature flags | Firmware build configuration in `BoardConfig.h`, `Console.h`, `Updates.h` and PlatformIO; not an SD preference |
| Games, game-specific hardware definitions, Lua, images, audio and portal assets | SD; these are portable application content |
| Browser language, developer mode and local audit display | Browser localStorage; not SD or the device's Wi-Fi password store |

Previously, `DeviceConfigRepository` combined SD defaults with UI/network NVS
overrides. A portal save also wrote the entire document, including the password,
to `/.core/config/config.json`. `NetworkSystem` had a separate `wifi.cfg` fallback.
Those SD write and runtime fallback paths have been removed. TFT and portal now
use the same repository and can save configuration with no SD card present.

## Migration

1. A valid `console-cfg/settings` is authoritative. SD files never override it,
   including after a card replacement or a configuration reset.
2. When that record is absent and SD is available, import
   `/.core/config/config.json`. Use `/.core/config/wifi.cfg` for legacy network
   credentials when the JSON does not contain a configured network.
3. Overlay existing `console-ui/prefs-v2` or older `theme`/`sound` keys, then
   `console-net/credentials`. Existing device preferences take priority over SD,
   including an explicitly cleared network. Italian remains the default language.
4. Commit the complete snapshot to NVS and reopen/read it to verify its contents.
   Only then remove the two obsolete SD files. This is a normal FAT deletion,
   not forensic secure erasure of data previously written to a card.
5. Interrupted migration, malformed/oversized inputs or a failed NVS write leave
   the SD originals intact. A failed deletion is logged and retried on a later
   repository load; the committed NVS settings remain usable.

Without SD, existing NVS settings still load. Reading defaults on a fresh device
does not mark SD migration complete. An explicit save/reset initializes NVS and
makes those user choices authoritative. Legacy NVS records are retained as an
upgrade fallback; they are not continually synchronized for older firmware.

An unreadable or unknown-version canonical record is not silently replaced from
SD. Partial UI/network saves fail instead of overwriting it. An explicit complete
save/reset can replace it. Failures never trigger an NVS partition erase.

## Persistence and limits

All changes use a single NVS blob and commit; partial UI or network changes merge
with the current snapshot under the repository lock. The RAM cache/revision is
updated for a save only after successful verification, and identical saves avoid
rewrites. When migration cannot finish, its loaded values may still be used in RAM
for that boot while the original files are retained for a later retry.
Theme/audio changes keep the existing 400 ms save debounce.

Device name and timezone are limited to 64 UTF-8 bytes, date format to 32,
brightness and channel volumes to 0–100. SSID/password validation remains 32/64
bytes, supporting open networks and 64-digit hexadecimal PSKs.

Reboot and ordinary application/OTA updates preserve the NVS partition. An erase
flash operation, explicit reset of configuration, or an incompatible partition
table can remove settings. This change does not alter the partition table or burn
eFuses. Switching back to firmware predating this migration may show older values
from the retained legacy NVS records; new changes are saved only in the new schema.

## Credential handling

The password is write-only in `/api/config`: responses return `wifi.password: null`
and `wifi.passwordConfigured`. This placeholder also preserves compatibility with
older portal builds that default a missing password to an empty string. Omitting
the password or passing null on a PUT for the same SSID preserves it; explicitly
passing `""` clears it for an open network. Changing SSID
without supplying a password does not reuse the previous network's password.
Configuration responses use `Cache-Control: no-store`.

The debug build uses Arduino log level 4: level 5's WebServer parser logs entire
request bodies, including submitted passwords. Application diagnostics never
export saved passwords or password hashes. SD release bundles exclude the legacy
configuration directory, which is also ignored by Git.

**Internal NVS is not encrypted in the current Arduino/ESP-IDF build.** This removes
the removable-SD copy but does not protect against physical extraction of device
flash. Hardware-backed encrypted NVS requires separate firmware/partition/key
provisioning; see [Espressif's NVS encryption documentation](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32s3/api-reference/storage/nvs_flash.html#nvs-encryption).
The portal still uses its existing local HTTP transport; this change does not
introduce TLS or authentication for portal administration.

## Verification

With debug diagnostics enabled and a free serial port:

```powershell
python scripts/diagnostics/ui_config_storage.py
python scripts/diagnostics/ui_network_connection.py
```

`ui config-test` uses only `cfg-check`, `cfg-ui-check` and `cfg-net-check` NVS
namespaces and RAM-backed legacy files. It checks complete persistence without SD,
partial updates, redaction, migration precedence, failed/invalid migrations,
cleanup retries and unsupported schemas. It never creates personal files on SD.

The script stores its reboot comparison baseline in `cfg-verify` on the device,
compares the full configuration after reset, and removes the baseline on success.
Only boolean results cross serial, and user preferences/statistics are compared.
The Wi-Fi regression backs up the complete snapshot in `wifi-check-v2` before
temporarily replacing credentials, restoring it even when a test fails.

Frontend checks:

```sh
cd frontend
npm run test:config
npm run build
```

After building the portal, copy `sd/.core/portal` to the physical SD as usual.
Uploading firmware does not copy portal assets.

Verified on 13 November 2026: **28 on-device checks passed**, complete settings
comparison across reset passed, and legacy configuration files were absent from
the real SD after migration. Real Wi-Fi tests passed for empty credentials,
unreachable networks, password cancellation/submission, automatic startup
connection and keyboard confirmation; the original snapshot was restored.
The three frontend contract tests (including a legacy-client round trip), frontend
build/type-check and ten packaging tests also passed.
