# Firmware updates from GitHub Releases

Open **Settings > System > Updates** (**Impostazioni > Sistema > Aggiornamenti**).
The screen displays the installed firmware version. Connect the console to a
Wi-Fi network with Internet access, press **Check**, read the available version
and release notes, then press **Install** and confirm. The console restarts
automatically after a successful installation.

The first firmware containing this client must be installed by USB. Later
compatible stable firmware releases can be installed from the console. The
portal is not required; `WIFI_ENABLED=1` and `OTA_ENABLED=1` in
[`Console.h`](../src/config/Console.h) are required. Network settings remain in
the existing Network screen. Checking for updates does not install anything.

## Release source and private repositories

[`Updates.h`](../src/config/Updates.h) selects `Matteo099/er-console` at build
time. The client requests GitHub's latest published, non-draft, non-prerelease
release. Feature branch Actions artifacts and development builds are not offered
as updates. Semantic versions are compared numerically, including prerelease
precedence; equal or older releases cannot be installed through this screen.

Public releases work without authentication. This repository is currently
private: use **GitHub access** in the Updates screen to enter a fine-grained
personal access token restricted to this repository with **Contents: read**.
Use a dedicated, expiring token. The field is masked, has a visibility button
and Cancel, and accepts up to 256 characters. **Remove token** switches back to
unauthenticated access. A private repository and an absent release can both
produce GitHub's 404 response, so the screen does not mistake this for an
up-to-date firmware.

The token is stored in the `ota-updates` NVS namespace, separate from SD/portal
configuration. It is never compiled into firmware, shown again in the editor,
exported in the SD bundle, included in diagnostics, or copied from PC Git
credentials. NVS encryption is not enabled by this project: someone with
physical flash access can extract stored credentials. The token is sent only
to `api.github.com`; redirected asset downloads start a new request without
the Authorization header. Only explicit GitHub HTTPS hosts are accepted.

There must be a stable release produced by the [release pipeline](CI-CD.md).
The OTA client does not create releases. Publishing/merging the pipeline and
the first stable release is a separate repository operation.

## Required release contract

The client consumes the existing pipeline's `manifest.json` asset and the
application `.bin` asset named in `files.firmware`. The manifest must have:

- `schema_version: 1`, `channel: "stable"`, `dirty: false`;
- a version matching the release tag;
- matching `hardware` and `platformio_environment` for this board;
- `firmware_type: "application"`;
- the same `partition_table_sha256` as the installed firmware;
- `ota_slot_size` matching the actual device's inactive slot;
- the exact asset filename, size and SHA-256 in `files.firmware`.

The asset must be uploaded, fit the inactive slot, have an ESP32-S3 application
header, and pass native ESP image validation. GitHub's asset digest is also
checked against the manifest when present. Merged factory images, another
board, changed partition tables, missing assets and oversized images are
rejected. The current board has two 6,553,600-byte OTA application slots in
16 MB flash. The build script embeds the board ID and hash of the partition
CSV used for that build; missing metadata fails closed.

Firmware OTA updates the application only. It does not update the bootloader,
partition table, games, SD assets or portal. Update SD files separately when a
release requires them; existing preferences, saves and statistics are retained.

## Progress, failures and recovery

The worker runs outside the UI task. Downloads are streamed in 4 KB chunks
directly into the inactive application slot; the complete binary is never
buffered in RAM or saved to SD. The screen reports both download and flash
write percentages, followed by verification, activation and reboot. Release
notes are plain readable text, bounded to 2 KB with a truncation notice.
Metadata is bounded to 64 KB and the manifest to 8 KB; large buffers use PSRAM.

HTTPS validates the server certificate using the SDK root bundle. A bounded
NTP synchronization establishes the clock before TLS when necessary. There is
no insecure-certificate fallback. I/O stalls time out after 15 seconds, each
blocking connection operation is bounded, and an update operation has a
10-minute limit. The UI reports network, clock, authentication, rate-limit,
metadata, memory, compatibility, integrity and flash errors separately.

Installation is refused while a game is loaded. Before erasing flash, audio
playback is stopped and the updater waits for the audio task to acknowledge
that I2S is stopped; the rainbow LED task is also suspended during writes.
Navigation and new playback are blocked during flashing. **Cancel** remains
available until final validation/activation; a cancelled or failed transfer
never changes the selected boot partition. Retry starts the download again;
partial download resumption is not implemented.

The complete streamed SHA-256 and native `esp_ota_end` validation must succeed
before the new partition is selected. The running application is not
overwritten. Interrupted downloads, failed writes and hash mismatches leave
the previous application bootable. This is integrity verification over
authenticated HTTPS, not independent firmware signing or Secure Boot.

The ESP-IDF rollback-enabled bootloader starts the new image pending
verification. The Arduino early-confirmation hook is overridden. After boot,
the client requires boot/UI/audio/Lua readiness, a live UI heartbeat and at
least 24 KB free internal RAM continuously for 3 seconds before confirming the
image. If health is not established within 45 seconds it requests rollback;
a reset/crash before confirmation also allows the bootloader to return to the
previous image. NVS records the intended version and successful/interrupted
outcome for the Updates screen. Confirmation does not require Internet access
or a mounted SD card. A power loss after activation is handled by the native
redundant OTA data records and rollback process, not by rewriting the old slot.

## Verification and development diagnostics

Build the debug environment with `UI_DIAGNOSTICS_ENABLED=1`, upload it and run:

```powershell
python scripts/diagnostics/ui_ota.py --port COM3 --output .pio/ota-check
```

This exercises SemVer, metadata and manifest validation, redirect/credential
policy, SHA-256, interrupted framing, cancellation, injected flash failures,
isolated credential storage and LVGL controls. Preview screens do not perform
an installation. The script also checks the real network/error path and return
to the System tab, while preserving preferences, statistics and credentials.

To additionally test real flash erase/write/native image validation using the
currently built application:

```powershell
python scripts/diagnostics/ui_ota.py --port COM3 --output .pio/ota-native `
  --native-image .pio/build/esp32-s3-devkitc-1-n16r8v-debug/firmware.bin
```

This diagnostic writes the inactive slot and validates the supplied image but
**does not select it for boot or reboot the device**. The script verifies that
boot selection is unchanged. The serial-only helper and synthetic preview
commands are excluded from normal builds. Restore `UI_DIAGNOSTICS_ENABLED=0`
after testing. End-to-end HTTPS installation and post-update rollback still
need an accessible stable release, an Internet-connected console and, for
this private repository, a suitable token.

Implementation: [`systems/update`](../src/systems/update) contains transport,
metadata validation, a separately testable streaming engine, NVS credentials
and the worker/boot verification service. [`scenes/update`](../src/scenes/update)
keeps UI and navigation in the existing View/Scene structure. All interface
messages use the central Italian/English translation catalog.

Native OTA behavior: [ESP-IDF 4.4.7 OTA documentation](https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32s3/api-reference/system/ota.html).
GitHub endpoints: [latest release](https://docs.github.com/en/rest/releases/releases#get-the-latest-release)
and [release assets](https://docs.github.com/en/rest/releases/assets#get-a-release-asset).
