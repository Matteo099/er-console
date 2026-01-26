#include "BootScene.h"

#include "BootScene.h"
#include "os/BootEvent.h"
#include "core/SystemEventBus.h"

void BootScene::onEnter() {
    view.create();
}

void BootScene::onUpdate() {
    BootEvent evt;
    while (xQueueReceive(SystemEventBus::bootQueue, &evt, 0)) {
        view.setProgress(evt.progress, evt.label);
    }
    view.update();
}

void BootScene::onExit() {
    view.destroy();
}
