#include "RainbowPixel.h"
#include "log/Logger.h"

namespace {
constexpr uint8_t MAX_BRIGHTNESS = 5;
constexpr uint8_t COLOR_SEGMENT = 85;
}  // namespace

RainbowPixel::RainbowPixel()
    : pixels(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800), offset(0) {
}

void RainbowPixel::init(void* params) {
    Log::info(Log::LED_TAG, "Initializing RainbowPixel (NUM=%d PIN=%d)", NEOPIXEL_COUNT,
              NEOPIXEL_PIN);

    pixels.begin();
    pixels.setBrightness(MAX_BRIGHTNESS);
    pixels.clear();
    pixels.show();
}

void RainbowPixel::update() {
    const uint16_t count = pixels.numPixels();

    for (uint16_t i = 0; i < count; ++i) {
        pixels.setPixelColor(i, wheel((i + offset) & 0xFF));
    }

    pixels.show();
    ++offset;
}

uint32_t RainbowPixel::wheel(uint8_t pos) const {
    pos = 255 - pos;

    if (pos < COLOR_SEGMENT) {
        return pixels.Color(255 - pos * 3, 0, pos * 3);
    }

    if (pos < 2 * COLOR_SEGMENT) {
        pos -= COLOR_SEGMENT;
        return pixels.Color(0, pos * 3, 255 - pos * 3);
    }

    pos -= 2 * COLOR_SEGMENT;
    return pixels.Color(pos * 3, 255 - pos * 3, 0);
}