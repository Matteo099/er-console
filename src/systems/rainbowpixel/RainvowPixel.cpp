#include "RainbowPixel.h"

RainbowPixel::RainbowPixel()
    : pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800) {}

void RainbowPixel::init(void *params)
{
  pixels.begin();
  pixels.setBrightness(5);
}

void RainbowPixel::update()
{
    static uint8_t j = 0;

    for (int i = 0; i < pixels.numPixels(); i++)
        pixels.setPixelColor(i, wheel((i + j) & 255));

    pixels.show();
    j++;
}

uint32_t RainbowPixel::wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;

  if (WheelPos < 85)
  {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170)
  {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }

  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
