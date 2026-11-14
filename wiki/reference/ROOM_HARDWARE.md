# Room hardware checks

The game declares its required devices in `config.json` under `inputs`.
Enabled devices are required; disabled devices are ignored. Hardware is checked
on the input task before audio/asset preload and Lua initialization, again before a new session or
resume, and periodically while the loaded game remains open. The UI stays on
the existing loading screen while a check runs.

Missing devices and configuration errors open a dedicated, scrollable hardware
view with **Retry** and **Back**. It names the device/address or configuration
field and explains what to check. Hardware errors do not display “Time expired”
or play the game-over sound. Rejected attempts do not create statistics sessions.
Failed checks precede asset/audio preload, so a disconnected room does not wait
for large WAV files to load before displaying the problem.

## Supported checks

| Requirement | Check |
| --- | --- |
| MCP23017 #1/#2, INA219, SSD1306, PCA9685 | I2C response at the configured address, then driver initialization |
| RFID MFRC522 | Known read-only version register and driver initialization on the configured SPI3 bus |
| Optional room identifier DS2401 | Presence, family code, ROM CRC and exact expected room ID |
| Configuration | Strict field types/ranges, supported features, unique I2C addresses, non-overlapping channel ranges, reserved/duplicate GPIOs |

I2C transfers have a 20 ms timeout. Background checks run every 500 ms; two
consecutive failures stop the game (typically within about one second, plus
bounded bus/driver time). A different, valid room ID stops it on the first
sample. Start/resume checks reject a failure immediately. The input task retains
its objects until the scene unloads Lua, then shuts the subsystems down. Relay
outputs are shut down before their channel providers are destroyed.
Disconnected I2C drivers skip repeated cleanup writes after a failed presence
check. MCP pins are released without first forcing all output latches LOW.
Outputs can only be controlled while their communication link still responds.

An interrupted session is recorded as **abandoned**, without adding a win or
loss. The timer and audio stop. **Retry starts a new game from the beginning**;
automatic resume is deliberately avoided because reconnected hardware may have
lost its output configuration and internal state.

I2C acknowledgement proves that an address responds, not that every wire or
every downstream sensor/actuator works. Direct buttons, relays, multiplexers,
write-only MAX7219 displays and LEDs have no presence feedback in this wiring.
Their physical presence cannot be inferred from successful GPIO writes.
Without a room identifier, rooms with identical device profiles are
indistinguishable, and a cable carrying only passive GPIO signals cannot be
reliably detected as unplugged. Use the identifier below for those cases.

## Identify a room with one GPIO

Fit **one DS2401 per room**. Its factory ROM contains a unique serial number
and CRC. It uses one ESP32 GPIO plus the common ground already carried by the
room cable; power comes from its 1-Wire data line.

Mount the chip **on the room side, beyond the detachable cable**, so unplugging
either end removes the identifier. A chip attached beside the console would
remain visible after the room cable was disconnected.

For the DS2401 **TO-92** package:

| Connection | Wiring |
| --- | --- |
| DS2401 DQ, pin 2 | Dedicated GPIO, for example GPIO 38 |
| DS2401 GND, pin 1 | Console/room common GND |
| DS2401 NC, pin 3 | Leave unconnected |
| Pull-up resistor | 4.7 kΩ between DQ and the console's **3.3 V** |

Use the package orientation in the manufacturer drawing when identifying pins.
The resistor is the bus pull-up; the chip supplies the room's identity. This
implementation expects a single DS2401 on this dedicated line and rejects
multiple devices, invalid CRCs or another family. Test the identifier through
the final cable; a failed transaction is reported as missing/unreliable identity.

Read the installed chip's ROM with debug diagnostics enabled, outside a game:

```text
ui room-id 38
@ROOMID pin=38 rom=...
```

Then add the returned **16 hexadecimal digits, in the printed byte order**, to
the room's own game configuration:

```json
{
  "inputs": {
    "connection": {
      "enabled": true,
      "pin": 38,
      "rom": "010100000000000A"
    }
  }
}
```

The ROM above is a synthetic test ID: replace it with the actual value. The
firmware validates family and CRC, normalizes hexadecimal letters, and reserves
the selected GPIO so game channel calls cannot reuse it. Leaving `connection`
absent/disabled keeps the identifier optional for existing rooms. The expected
ROM is part of the room's hardware contract, not a Wi-Fi credential or console
preference. For release-bundled examples, create a game with your own directory
ID before editing hardware settings: official release resources are read-only,
and [SD updates](SD_UPDATES.md) preserve custom games with distinct IDs.

The ID is a connection/selection check, not cryptographic authentication. It
does not establish that every component downstream of the tag is working.

## Buses, pin conflicts and unsupported games

The current N16R8 board exposes native room GPIOs 1, 2, 19 and 38–42, excluding
any of those assigned to console functions in `BoardConfig.h`. Console buses,
display/touch, audio, status LED, UART, flash/PSRAM and boot pins are reserved.
Game channels cannot reconfigure those pins, the room ID line or pins already
assigned to another configured peripheral.

The I2C bus uses `I2C_SDA_PIN`, `I2C_SCL_PIN`, `I2C_FREQUENCY` from the board
configuration. Per-game overrides that differ are rejected. Supported MCP
channel ranges must start above native GPIOs, end by channel 255, and not
overlap another provider. A relay requires enabled channels.

RFID uses `"spiBus": "sd"` (the default): SPI3/HSPI, sharing SCK/MISO/MOSI
with SD, with separate CS and RST. Example control pins are CS 41, RST 40.
The RFID library now receives this actual `SPIClass`; the previous driver
passed it to a library that instead used global `SPI`. TFT/SPI2 RFID wiring
is not supported by the current bus integration and is explicitly rejected.
The MAX7219 driver also establishes its SPI transaction settings.

The current firmware supports MCP #1/#2, not MCP #3/#4 or the proposed ADC,
servo, TM1637 and LED-strip APIs. A game requesting these is rejected with an
unsupported-feature message instead of running partially. The bundled
`2-anno_2039_centrale_nucleare_dolomiti` describes that future contract. The
`1-lab` example also contains conflicts with this board's pins/addresses;
correct its hardware profile before using it. These failures must not be
misdiagnosed as SD read errors.

## Verification

With `UI_DIAGNOSTICS_ENABLED=1` and the debug environment uploaded:

```sh
python scripts/diagnostics/ui_hardware.py --port COM3 --output .pio/ui-hardware
```

The probe requires `3-t_loop` on the SD. It tests the parser/checker with a fake
probe (present/missing devices, room match/mismatch, bad CRC, bus/pin conflicts,
unsupported declarations and transient failures), then exercises real scenes
in Italian and English. The real missing-I2C case uses address 0x27; it must be
absent for that case. Disconnection during play is explicitly injected, not
claimed as a physical cable test. User statistics use the existing diagnostic
sandbox, preferences are not rewritten, and SD games are not modified.

`ui hardware-state` prints state/issue count and whether Lua is loaded.
`ui hardware-game missing|invalid|working|identity` creates a test scene;
`ui hardware-fault` injects a fault into the active test game. These commands
are excluded from normal builds. `identity` probes GPIO 38 and should only be
used when that line is reserved for the planned 1-Wire connection.

After fitting the chip, also verify the real hardware: correct room starts,
another room is rejected, unplugging during play stops it, and a missing
required I2C/RFID component is named correctly. Restore diagnostics to zero
after testing. A simulated ROM test does not validate the physical 1-Wire bus.

Verified on 14 November 2026: 28 checker/layout tests passed on ESP32, followed
by real missing-I2C and invalid-pin scenes, retry/back in Italian and English,
and an injected in-game disconnect. Failed rooms were rejected in 552–555 ms,
before any asset preload, versus about 7–8 seconds with the previous ordering.
Lua and statistics remained inactive for rejected attempts; the injected fault
stopped the running session without a loss. The existing 13 in-game settings
checks and the real Loop game's load, sliders, pause/resume and natural timeout
also passed with original preferences/statistics restored. Debug and release
builds succeeded. Logs/screenshots: `.pio/hardware-early-verified/` and
`.pio/hardware-final-ingame/`. Successful physical DS2401/RFID identification
and a real cable unplug remain hardware acceptance tests after fitting and
connecting those devices; the software tests do not claim those results.

Sources: [DS2401 data sheet](https://www.analog.com/media/en/technical-documentation/data-sheets/DS2401.pdf?R-C01=),
[MFRC522 data sheet](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf),
[RFID SPI transport](https://github.com/OSSLibraries/Arduino_MFRC522v2/blob/2.0.6/src/MFRC522DriverSPI.h),
[OneWire](https://github.com/PaulStoffregen/OneWire).
