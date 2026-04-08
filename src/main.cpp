#include <Arduino.h>
#include "log/Logger.h"
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "refactor/LGFX.h"
#include "refactor/LVGLManager.h"

// ---------------------------
// Create display object
// ---------------------------
LGFX display;

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing display + touch + SD...");

  // TFT + touch
  display.init();
  display.setRotation(1);
  Serial.println("Display initialized");

  LVGLManager manager(display);
  manager.init();
}

void loop()
{
  lv_timer_handler();
  delay(5);
}