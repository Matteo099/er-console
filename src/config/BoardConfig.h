#pragma once

// --- TFT (VSPI) pins ---
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST   4
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// --- Touch (HSPI) pins ---
#define TOUCH_CS   10
#define TOUCH_IRQ  36
#define TOUCH_MOSI 45
#define TOUCH_MISO 39
#define TOUCH_SCLK 20

// sd
#define SD_CS_PIN 5

// audio
#define I2S_BCLK  18
#define I2S_LRC   17
#define I2S_DOUT  16
#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 44100


// neo-pixel
#define PIN 48
#define NUMPIXELS 1