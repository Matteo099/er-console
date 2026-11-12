#pragma once

#include <Arduino.h>
#include "SDCardDriver.h"
#include "StorageRecoveryStore.h"
#include "StorageRecoveryState.h"

class StorageRecoverySystem {
  public:
    explicit StorageRecoverySystem(SDCardDriver* driver, uint8_t relayPin,
                                   bool relayActiveHigh = true);

    bool recover(StorageFailureReason reason = StorageFailureReason::MountFailed,
                 bool restart = false);

    StorageRecoveryState lastFailure();

  private:
    void init();
    bool softRecover();
    bool powerCycleStorage();
    void printLastFailure();

    static const char* reasonToString(StorageFailureReason reason);

  private:
    bool initialized = false;
    uint8_t relayPin;
    bool relayActiveHigh;
    SDCardDriver* driver;
    StorageRecoveryStore store;
};