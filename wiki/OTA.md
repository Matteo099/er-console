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

## Public release source

[`Updates.h`](../src/config/Updates.h) selects `Matteo099/er-console` at build
time. The client requests GitHub's latest published, non-draft, non-prerelease
release. Feature branch Actions artifacts and development builds are not offered
as updates. Semantic versions are compared numerically, including prerelease
precedence; older releases cannot be installed through this screen. For the
installed stable version, **Repair SD** is available if its matching resources
are missing or incomplete. Repair reinstalls resources and reboots without
rewriting the firmware.

The repository and release assets must be public. All requests are anonymous;
the firmware has no access-token configuration, credential storage or login UI.
The latest release is retrieved from GitHub's public API, and the manifest and
firmware are downloaded from its release assets. HTTPS redirects to the GitHub
asset CDN are supported. A missing repository, release or asset produces a
public-release-unavailable message rather than an up-to-date result.

There must be a stable release produced by the [release pipeline](CI-CD.md).
The OTA client does not create releases. Publishing/merging the pipeline and
the first stable release is a separate repository operation.

## Required release contract

The client consumes the existing pipeline's `manifest.json` asset and the
application `.bin` asset named in `files.firmware`, and the streaming `.sdpack`
asset named in `files.resources`. The manifest must have:

- `schema_version: 2`, `channel: "stable"`, `dirty: false`;
- a version matching the release tag;
- matching `hardware` and `platformio_environment` for this board;
- `firmware_type: "application"`;
- the same `partition_table_sha256` as the installed firmware;
- `ota_slot_size` matching the actual device's inactive slot;
- the exact asset filename, size and SHA-256 in both `files.firmware` and
  `files.resources`.

The asset must be uploaded, fit the inactive slot, have an ESP32-S3 application
header, and pass native ESP image validation. GitHub's asset digest is also
checked against the manifest when present. Merged factory images, another
board, changed partition tables, missing assets and oversized images are
rejected. The current board has two 6,553,600-byte OTA application slots in
16 MB flash. The build script embeds the board ID and hash of the partition
CSV used for that build; missing metadata fails closed.

OTA installs the matching games, scripts, images, audio and compiled portal
before activating the application. The bootloader and partition table are not
updated. Existing preferences, saves and statistics are retained. See
[SD resource synchronization](reference/SD_UPDATES.md) for the managed directory
layout, custom games, provisioning and recovery.

## Progress, failures and recovery

The worker runs outside the UI task. SD resources are streamed into a reserved
staging directory, with per-file SHA-256 verification and a complete readback
from SD. The full pack digest is verified before publishing the new generation.
SD errors or insufficient space stop the update before firmware activation.
The UI displays a separate SD phase followed by the firmware phase.
Firmware downloads are streamed in 4 KB chunks
directly into the inactive application slot; the complete binary is never
buffered in RAM or saved to SD. The screen reports both download and flash
write percentages, followed by verification, activation and reboot. Release
notes are plain readable text, bounded to 2 KB with a truncation notice.
Metadata is bounded to 64 KB and the manifest to 8 KB; large buffers use PSRAM.

HTTPS validates the server certificate using the SDK root bundle. A bounded
NTP synchronization establishes the clock before TLS when necessary. There is
no insecure-certificate fallback. I/O stalls time out after 15 seconds, each
blocking connection operation is bounded, and an update operation has a
10-minute limit. The UI reports network, clock, rate-limit,
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
HTTPS with server-certificate validation, not independent firmware signing or Secure Boot.

The ESP-IDF rollback-enabled bootloader starts the new image pending
verification. The Arduino early-confirmation hook is overridden. After boot,
the client requires boot/UI/audio/Lua readiness, a live UI heartbeat and at
least 24 KB free internal RAM continuously for 3 seconds before confirming the
image. Release builds also require the matching SD generation. If health is not
established within 45 seconds it requests rollback;
a reset/crash before confirmation also allows the bootloader to return to the
previous image. NVS records the intended version and successful/interrupted
outcome for the Updates screen. Confirmation does not require Internet access;
it does require valid SD resources when a content hash is embedded. A power loss
after activation is handled by the native
redundant OTA data records and rollback process, not by rewriting the old slot.

## Verification and development diagnostics

Build the debug environment with `UI_DIAGNOSTICS_ENABLED=1`, upload it and run:

```powershell
python scripts/diagnostics/ui_ota.py --port COM3 --output .pio/ota-check
```

This exercises native SD staging and readback in a small isolated diagnostic
generation, SemVer, metadata and manifest validation, allowed HTTPS hosts,
SHA-256, interrupted framing, cancellation, injected flash failures and LVGL
controls, including the absence of an update credential editor. Preview screens do not perform
an installation. The script also checks the real network/error path and return
to the System tab, while preserving preferences and statistics.

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
need a public stable release and an Internet-connected console.

Historical verification before schema 2: on 11 November 2026, the public client passed 37 on-device engine/UI checks.
The ESP32 anonymously retrieved release `v1.0.0` and downloaded/validated its
manifest, including board, partition layout and firmware asset metadata.
The installed development version `1.0.1-dev...` was newer, so the UI correctly
disabled installation of `1.0.0`. No downgrade or boot-partition change was
performed. Logs/screenshots: `.pio/ota-public-check/`. Schema 1 releases are
now rejected; publish a release using the current pipeline to enable the
combined firmware/SD update contract.

Implementation: [`systems/update`](../src/systems/update) contains transport,
metadata validation, a separately testable streaming engine
and the worker/boot verification service. [`scenes/update`](../src/scenes/update)
keeps UI and navigation in the existing View/Scene structure. All interface
messages use the central Italian/English translation catalog.

Native OTA behavior: [ESP-IDF 4.4.7 OTA documentation](https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32s3/api-reference/system/ota.html).
GitHub endpoints: [latest release](https://docs.github.com/en/rest/releases/releases#get-the-latest-release)
and [release assets](https://docs.github.com/en/rest/releases/assets#get-a-release-asset).
