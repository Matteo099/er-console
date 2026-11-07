#pragma once

/*
ESP32-S3-N16R8 Board Configuration

MCU: ESP32-S3
Flash: 16 MB
PSRAM: 8 MB

This file centralizes ALL hardware pin assignments used by the
project. Changing a pin here automatically updates the whole project.

--------------------------------------------------------------------
PHYSICAL PINOUT (as printed on board)
--------------------------------------------------------------------

USB connectors at the bottom of the board.

TOP ROW (left → right)
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
| GND | TX  | RX  |  1  |  2  | 42  | 41  | 40  | 39  | 38  | 37  | 36  | 35  |  0  | 45  | 48  | 47  | 21  | 20  | 19  | GND | GND |
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+

BOTTOM ROW (left → right)
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+
| 3V3 | 3V3 | RST |  4  |  5  |  6  |  7  | 15  | 16  | 17  | 18  |  8  |  3  | 46  |  9  | 10  | 11  | 12  | 13  | 14  | 5Vi | GND |
+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+

Notes
-----
TX/RX : UART0 serial interface
RST   : Reset button
0     : Boot mode pin (affects flashing mode)
45/46 : Strapping pins (use with care)
5Vi   : USB power input
3V3   : Regulated 3.3V output
GND   : Ground

Not all pins are safe for general use. Some are used for boot mode or USB functionality. Pins chosen below avoid those conflicts.

PERIPHERAL BUS OVERVIEW

SPI2 HOST  → TFT display + Touch controller (SPI1_* pin macros below)
SPI3 HOST  → MicroSD card (Arduino HSPI)
I2S BUS    → Audio amplifier (MAX98357A)
I2C BUS    → Sensors / small displays
GPIO       → LEDs, buttons, actuators

*/


/* ================================================================
   SPI BUS 1 — TFT DISPLAY + TOUCH
   ================================================================

   Devices:
   - ILI9341 TFT Display
   - XPT2046 Touch Controller

   These devices share the same SPI bus but use separate
   Chip Select pins.

   SPI Mode: MODE0
   Typical Clock:
   - TFT: up to 40 MHz
   - Touch: ~2.5 MHz
*/

#define SPI1_MOSI_PIN        11
#define SPI1_MISO_PIN        13
#define SPI1_SCLK_PIN        12

/* TFT Display Control Pins */

#define TFT_CS_PIN           10   // Chip Select for TFT display
#define TFT_DC_PIN           9    // Data/Command control
#define TFT_RST_PIN          8    // Hardware reset line
// #define TFT_BL_PIN           3 // Backlight control (connect to 3.3V or it could be connected to GPIO for PWM brightness control)

/* Touch Controller */

#define TOUCH_CS_PIN         7    // Chip Select for touch controller
#define TOUCH_IRQ_PIN        6    // Touch interrupt (optional)



/* ================================================================
   SPI BUS 1 — MICRO SD CARD
   ================================================================
*/

#define SPI3_MISO_PIN        20  // MISO for SD; also native USB D+, use USB-UART for diagnostics
#define SPI3_SCLK_PIN        21  // SCLK pin for SPI3 (SD card)
#define SPI3_MOSI_PIN        47   // MOSI pin for SPI3 (SD card)
#define SD_CS_PIN            5   // SD card chip select
#define SD_POWER_RELAY_PIN   3   // Relay to control SD card power (optional)



/* ================================================================
   I2S AUDIO BUS — MAX98357A
   ================================================================

   Device:
   - MAX98357A I2S DAC + Class-D amplifier

   This chip converts digital I2S audio directly into speaker output.

   Connections:
   ESP32 I2S → MAX98357A
*/

#define I2S_PORT             I2S_NUM_0

#define I2S_DOUT_PIN         16     // Serial audio data (DIN on amplifier)
#define I2S_BCLK_PIN         18     // Bit clock (BCLK)
#define I2S_LRCLK_PIN        17     // Left/Right channel clock (LRC - WS)
#define I2S_SHUTDOWN_PIN      4     // SD card shutdown control (SD)

#define AUDIO_SAMPLE_RATE    22050  // Console PCM output: mono, 16 bit, 352800 bit/s

/*
GAIN pin: (do not connect)
- FLOAT → medium gain (default)
- GND   → low gain
- VIN   → high gain
*/

/* ================================================================
   I2C BUS — SENSORS / SMALL DISPLAYS
   ================================================================

   Shared bus for multiple devices such as:

   - BME280 environmental sensor
   - MPU6050 IMU
   - SSD1306 OLED display
*/

#define I2C_SDA_PIN          14
#define I2C_SCL_PIN          15

#define I2C_FREQUENCY        400000    // 400 kHz I2C; unrelated to the SD SPI clock



/* ================================================================
   NEOPIXEL / STATUS LED
   ================================================================

   Many ESP32-S3 boards include a built-in WS2812 RGB LED.
*/

#define NEOPIXEL_PIN         48
#define NEOPIXEL_COUNT       1



/* ================================================================
   FREE GPIOs FOR EXPANSION
   ================================================================

   These pins are currently unused and available for:

   - Buttons
   - LEDs
   - Relays
   - Servos
   - Analog sensors
   - Encoders
*/

#define GPIO_FREE_1          1
#define GPIO_FREE_2          2
#define GPIO_FREE_5          21  // Reserved by SD SCLK; not available while SD is used
#define GPIO_FREE_6          38
#define GPIO_FREE_7          39
#define GPIO_FREE_8          40
#define GPIO_FREE_9          41
#define GPIO_FREE_10         42
#define GPIO_FREE_11         47  // Reserved by SD MOSI; not available while SD is used
#define GPIO_MIN_PIN         0
#define GPIO_MAX_PIN         48



/* ================================================================
   PROJECT CONSTANTS
   ================================================================ */

#define DEFAULT_SPI_CLOCK    40000000   // 40 MHz SPI for TFT
#define TOUCH_SPI_CLOCK      2500000    // 2.5 MHz for touch controller
#define SD_SPI_CLOCK         4000000    // 4 MHz SD SPI; verified separately from I2C



/* ================================================================
   POWER NOTES
   ================================================================

   Approximate current draw:

   ESP32-S3           80–240 mA
   TFT backlight      60–100 mA
   MAX98357A audio    up to 300 mA peak

   Recommended power supply:
   5V @ ≥1A
   
   Recommended:
   - 100uF capacitor near TFT
   - 100uF capacitor near SD
   - 100uF capacitor near amplifier
*/
