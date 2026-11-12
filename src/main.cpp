#include <Arduino.h>
#include "os/EscapeOS.h"

EscapeOS os;

void setup() {
    os.init();
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}
