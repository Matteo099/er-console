#include "BootScene.h"

BootScene::BootScene(Display* d) : display(d), view(d) {}

void BootScene::onEnter() {
    display->clear(0x0000);  // pulizia display base
    view.create();            // crea la UI LVGL
}

void BootScene::onUpdate() {
    BootEvent evt;
    while (xQueueReceive(EscapeOS::bootQueue, &evt, 0) == pdTRUE) {
        progress = evt.progress;
        label = evt.label;
    }

    // aggiorna solo la UI, non ridisegna tutto a mano
    view.update(progress, label);
}

void BootScene::onExit() {
    // eventuale pulizia se necessaria
}
