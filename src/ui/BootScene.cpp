#include "BootScene.h"
#include "os/EscapeOS.h"
#include "os/BootEvent.h"

BootScene::BootScene(Display* d) : display(d) {}

void BootScene::onEnter() {
    display->clear(0x0000);
}

void BootScene::onUpdate() {

    BootEvent evt;
    while (xQueueReceive(EscapeOS::bootQueue, &evt, 0) == pdTRUE) {
        progress = evt.progress;
        label = evt.label;
    }

    render();
}

void BootScene::onExit() {}

void BootScene::render() {

    display->clear(0x0000);

    display->drawText(20, 20, "Escape Room Console", 0xFFFF);
    display->drawText(20, 50, label, 0xFFFF);

    display->drawRect(20, 100, 200, 20, 0xFFFF);

    int fill = (progress * 196) / 100;
    display->fillRect(22, 102, fill, 16, 0x07E0);
}
