# Room hardware checks

Games declare required hardware in `config.json` under `inputs`. Enabled devices
are checked on the input task before asset/audio preload and Lua initialization,
again before starting/resuming a session, and every 500 ms while the game is open.
Missing hardware opens a translated error view with Retry and Back; it never
appears as a timeout, and rejected starts do not create a statistics session.

## Identify the game with one ADC input

Use a resistor to identify the **game/type of room**. Every clone of game A uses
the same resistor and the same game configuration; replacing a room does not
require assigning a new identifier. Different games need well separated values.
This check verifies the connection/type, not the identity of an individual unit.

The console provides a **1 kOhm, 1%** pull-up to **3.3 V**. Fit the identifying
resistor **in the room, beyond the detachable cable**, between the signal and
common GND. One signal GPIO is needed, in addition to the existing common ground.
Do not connect this line to 5 V or use the ESP32 internal pull-up.

```text
CONSOLE                              ROOM (beyond cable)
3.3 V -- 1 kOhm, 1% --+---- cable ---- Rx, 1% ---- GND
                     |
                 GPIO1 (ADC1)
                     |
                   100 nF
                     |
                    GND ------------- common GND
```

The capacitor is console-side, close to the ADC pin. GPIO1 is the default;
GPIO2 is the only other supported free ADC1 pin on this board. GPIO38 is not
an ADC input. The selected pin is reserved against all other game peripherals
and channel operations. ADC1 also avoids the ADC2/Wi-Fi ownership conflict.

The divider produces `V = 3300 * Rx / (1000 + Rx)` millivolts. For example:

| Game/type | Room resistor | Nominal ADC voltage | Accepted voltage |
| --- | ---: | ---: | ---: |
| A | 220 Ohm | 595 mV | 520-667 mV |
| B | 380 Ohm | 908 mV | 815-997 mV |
| B, standard alternative | 390 Ohm | 925 mV | 832-1015 mV |
| C | 680 Ohm | 1335 mV | 1227-1437 mV |
| D | 1 kOhm | 1650 mV | 1538-1753 mV |
| E | 2.2 kOhm | 2268 mV | 2167-2360 mV |
| F | 4.7 kOhm | 2721 mV | 2644-2790 mV |

Choose **either 380 or 390 Ohm for B**, never for two different game types: their
windows overlap. The firmware compares only the selected game's expected value;
it cannot detect overlapping codes in another game's configuration. Likewise,
identical resistors deliberately authorize the same game on every clone. Use a
shared allocation table when adding games; leave gaps between voltage windows.
The supported resistance range is 150-4700 Ohm. The acceptance window uses
resistance +/-10%, plus 25 mV on each side for ADC error. Use 1% resistors anyway,
and verify readings with the final cable and power supply. These margins allow
normal variation; they do not make arbitrary adjacent resistor values distinct.

Configure each game, for example game A:

```json
{
  "inputs": {
    "connection": {
      "enabled": true,
      "pin": 1,
      "resistanceOhms": 220
    }
  }
}
```

`resistanceOhms` is required when enabled and must be an integer. When disabled
or omitted, games do not require an identification resistor. No unique serial
number, registry, enrollment or migration is involved. This is public game
hardware metadata, not a personal/device preference. To customize official
release games, copy to a distinct game directory ID: [SD updates](SD_UPDATES.md)
preserve custom games with distinct IDs.

The input is sampled with the core's calibrated `analogReadMilliVolts`, 12-bit
resolution and 11 dB attenuation. Each check takes 32 samples, drops the four
highest and lowest, and averages the remainder. A retained spread above 80 mV
reports unstable contacts. Stable readings >=3000 mV report an open cable or
missing resistor; <=120 mV report a short. Other out-of-window readings report
an incorrect room. All of these block further hardware initialization.
The divider, including its external pull-up, is mandatory: a floating input
cannot reliably establish cable presence. An open resistor or broken ground
looks like an unplugged cable; this circuit cannot distinguish those causes.

With debug diagnostics enabled, outside a game, inspect the wired divider:

```text
ui room-resistor 1
@ROOMRESISTOR pin=1 mv=595 spread=8 ohms=219 state=measured
```

`ohms` is an estimate using nominal 3.3 V and 1 kOhm; only `state=measured`
represents a usable resistance estimate. Other states are `open`, `short`, or
`unstable`. Compare real readings against the table rather than widening the
windows until a wrong room is accepted. Test every clone and unplug either end
of the cable. A correct resistor does not prove downstream components work.

## Other hardware and runtime behavior

| Requirement | Check |
| --- | --- |
| MCP23017 #1/#2, INA219, SSD1306, PCA9685 | I2C acknowledgement at the configured address, then driver initialization |
| RFID MFRC522 | Known read-only version register, then driver initialization on SPI3 |
| Configuration | Strict types/ranges, supported features, unique I2C addresses, disjoint channel ranges, reserved/duplicate GPIOs |

I2C transfers have a 20 ms timeout. Two consecutive background failures stop a
session (usually about one second, plus bus time); a stable wrong-room reading
stops it on its first sample. Start/resume checks reject failures immediately.
An interrupted session is recorded as abandoned, without adding a win or loss;
timer/audio stop, and Retry reloads from the beginning. Relay outputs are shut
down before their providers are destroyed. Disconnected I2C drivers avoid
repeated cleanup writes; outputs can only be controlled while their link works.

I2C acknowledgement proves an address responds, not every component's identity
or all downstream wiring. Passive buttons, relays, multiplexers, write-only
MAX7219 displays and LEDs have no presence feedback. GPIO writes cannot prove
their connection. With no identification resistor, equal component profiles
remain indistinguishable and a passive GPIO cable cannot be reliably detected.

Native room GPIOs on this N16R8 board are 1, 2, 19 and 38-42, excluding any
assigned to console functions in `BoardConfig.h`. UART, flash/PSRAM, boot pins,
and console buses are reserved. Only 1/2 may serve as the identification ADC.
I2C uses the board's SDA/SCL/frequency; incompatible game overrides are rejected.
MCP channel ranges must start above native GPIOs, end by 255, and not overlap.
Relays require channels. RFID uses `"spiBus": "sd"`: SPI3/HSPI with separate
CS/RST (for example 41/40); SPI2/TFT is not supported. RFID receives the actual
shared `SPIClass`, and MAX7219 establishes its SPI transaction settings.

MCP #3/#4 and the proposed general ADC, servo, TM1637 and LED-strip APIs are
unsupported. The identification ADC above is independent of those proposed game
APIs. The bundled nuclear-room example requests unsupported devices; `1-lab`
has board pin/address conflicts. Correct their profiles before using them;
these configuration errors are not SD read failures.

## Verification

With `UI_DIAGNOSTICS_ENABLED=1` and the debug environment uploaded:

```sh
python scripts/diagnostics/ui_hardware.py --port COM3 --output .pio/ui-hardware
```

This tests the parser/checker with simulated readings: matching clones, wrong
resistor, tolerance boundaries, open/short/noisy line, invalid ADC pins/values,
conflicts, missing components and transient failures. It then exercises real
scenes in Italian and English, Retry/Back, and an injected in-game disconnect.
The missing-I2C case requires address 0x27 to be absent; `3-t_loop` must be on SD.
Statistics use the diagnostic sandbox; preferences and SD games are preserved.
`--core-only` runs the model/layout tests without creating sessions.

`ui hardware-state` prints state/issue count and Lua status.
`ui hardware-game missing|invalid|working|identity` opens a diagnostic game;
`identity` expects 220 Ohm on GPIO1 and requires the external divider first.
`ui hardware-fault` injects a disconnect into a running diagnostic game.
These commands are excluded from normal builds. Restore diagnostics to zero
following tests. Simulated readings do not validate a physical resistor/cable.
After wiring, acceptance tests must cover correct/wrong room, multiple clones,
short/open/noisy contacts, unplug during play, and missing required I2C/RFID.

Verified on ESP32 on 14 November 2026: **49 checker/layout tests passed**.
The integration probe passed missing-device/invalid-pin scenes in IT/EN,
Retry/Back, and an injected in-game disconnect without adding a loss. Rejected
starts took 560-577 ms and did not load assets or start Lua/statistics.
The ADC diagnostic also rejected GPIO38 and returned a reading from GPIO1.
Logs/screenshots: `.pio/resistor-hardware/` and `.pio/resistor-adc/`.
Debug and release builds passed; the final debug firmware was uploaded with
diagnostics disabled. Boot capture confirmed SD ready, main menu and Wi-Fi IP
assignment (`.pio/resistor-final-boot.log`).
No identifying resistor/divider was fitted for these tests: the physical
matching-clone, wrong-resistor and cable-unplug tests remain to be performed.

Sources: [Espressif ADC API](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html),
[ESP32-S3 ADC noise and filtering](https://docs.espressif.com/projects/esp-idf/en/v4.4.3/esp32s3/api-reference/peripherals/adc.html),
[MFRC522 data sheet](https://www.nxp.com/docs/en/data-sheet/MFRC522.pdf),
[RFID SPI transport](https://github.com/OSSLibraries/Arduino_MFRC522v2/blob/2.0.6/src/MFRC522DriverSPI.h).
