# SD resources synchronized with firmware

Release firmware identifies its complete SD distribution by the SHA-256 of the
`.sdpack` asset, embedded at build time as `SD_CONTENT_SHA256`. The pipeline
prepares the portal and assets before compiling, then verifies that the same
bytes are packaged afterward. The release manifest uses schema 2 and includes
`files.resources` (filename, exact byte count, SHA-256).

## Installation and recovery

1. Settings checks the public release and validates the board, version, layout
   and both firmware/resource asset records.
2. The updater stops audio and streams resources into `/.releases/staging`.
   It needs free space for the complete new generation plus FAT allocation
   overhead (32 KiB per file and a 1 MiB reserve). Packages are limited to
   128 MiB and 1,024 files; the full package is never held in ESP32 RAM.
3. Every file is hashed, flushed, closed and reread from SD for SHA-256
   verification. The package SHA-256 and exact HTTP framing must also match.
4. The completed directory is renamed to `/.releases/<package-sha256>/`.
   Only after this succeeds does firmware download/validation/activation run.
5. On reboot, the firmware selects its embedded content hash. The completion
   marker binds version, hash and hardware; the inventory checks every required
   file's presence and size. OTA boot confirmation additionally requires a
   matching generation.

The previous generation is retained for firmware rollback. Cancelling, losing
network access or failing an SD write leaves the existing firmware and its
generation selected. A power interruption before activation leaves the old
firmware selected. An interrupted staging directory is removed on retry only
when its updater ownership record is present. An unrecognized staging directory
is left untouched and reported as an SD error. A corrupt existing target is
retained with a `-replaced-<uptime>` suffix when a repair publishes its replacement.

FAT operations are not a journaled transaction; filesystem corruption after a
power loss can still require card repair. Never remove the card during updates.
Old generations are not automatically garbage-collected: free space on a PC
if needed, keeping both the current and previous firmware's generations.

## User data and logical paths

The storage driver keeps logical paths such as `/.core/portal/index.html` and
`/games/example/game.lua` stable. Reads select the matching generation first;
directory listings merge user games from the card's ordinary `/games` directory.
The reserved `.releases` tree is hidden from portal file browsing and protected
against ordinary file writes, moves and deletes.

- Release `.core` resources and bundled game files are read-only through the
  application. Removed official files do not persist inside a new generation.
- User files outside the managed tree are never removed by the updater.
  Runtime saves under a game directory continue to use the ordinary flat path.
- Custom games use distinct directory IDs under `/games`. If a custom game has
  the same ID/path as an official game, the release file takes precedence;
  the original flat file remains physically untouched. Copy modified examples
  to a unique game ID to keep them editable and independent.
- Device preferences and Wi-Fi credentials remain in internal NVS; they are
  excluded from the SD package. NVS statistics are also unaffected.

If a release generation is absent or incomplete, games display a localized
message directing the user to Updates. Built-in UI resources keep Settings
available. Missing portal files return an HTTP error instead of serving an
older portal. With Internet available, check Updates and install the latest
release. When the latest stable release equals the installed version and its
embedded content hash matches, **Repair SD** reinstalls resources without
flashing firmware. The console reboots afterward to discard cached assets.

Boot checks detect missing/truncated files, not every possible same-length bit
change. SHA-256 verification covers all downloaded data and the complete SD
readback during installation. A physically faulty card must be replaced.

## USB provisioning and development

For a release installed over USB, extract the matching `...-sd.zip` into the
card root, including hidden `.releases` files. The ZIP contains the same bytes
as OTA plus completion/inventory records. Do not flatten its directory layout.

`python scripts/ci/build_bundle.py --channel dev` also embeds a content hash and
produces a matching SD ZIP. Ordinary standalone PlatformIO builds omit the hash
and read the flat repository `sd/` layout for development. No migration or
compatibility adapter is used. Schema 1 releases are not accepted by this client.

## Verification

`python -m unittest discover -s scripts/ci -p 'test_*.py'` checks deterministic
packaging, per-file hashes, unsafe paths, truncation and consistency of SD ZIP
and streaming pack. With debug diagnostics enabled, run
`python scripts/diagnostics/ui_ota.py --port COM3 --output .pio/sd-ota-check`.
The device tests fragmented streams, corruption, cancellation, failures and
actual SD write/readback in an isolated diagnostic generation. They do not
activate firmware or publish a GitHub release. The diagnostic generation is
small and retained under its own hash; it is never selected by release firmware.

Implementation: [pack format](../../scripts/ci/sd_pack.py),
[SD installer](../../src/systems/update/SdResources.cpp),
[OTA worker](../../src/systems/update/UpdateSystem.cpp).

Verified on 13 November 2026: the final ESP32 run passed 56 checks, including
native staging cancellation, file readback, missing-file detection, protected
resources and flat runtime saves. UI navigation, preferences and statistics
preservation also passed. The SD diagnostic worker retained 4,076 bytes of free
stack. Logs and screenshots: `.pio/sd-ota-verified/`. The complete development
bundle (firmware, Vue portal, 38 SD files) was built and verified locally,
including the content hash embedded in the binary. Public GitHub installation
and reboot/rollback of a new schema-2 release remain an end-to-end release test;
the existing schema-1 public release is correctly rejected.
