#include "StorageRecoveryState.h"

#include <Preferences.h>

class StorageRecoveryStore {
  public:
    void save(StorageFailureReason reason) {
        Preferences prefs;
        prefs.begin("storage", false);
        uint32_t count = prefs.getUInt("failures", 0);
        prefs.putUChar("reason", static_cast<uint8_t>(reason));
        prefs.putUInt("failures", count + 1);
        prefs.putUInt("boot", prefs.getUInt("boot", 0) + 1);
        prefs.putUInt("time", millis());
        prefs.end();
    }

    StorageRecoveryState load() {
        Preferences prefs;
        prefs.begin("storage", true);
        StorageRecoveryState state;
        state.reason = static_cast<StorageFailureReason>(prefs.getUChar("reason", 0));
        state.failureCount = prefs.getUInt("failures", 0);
        state.bootCount = prefs.getUInt("boot", 0);
        state.timestamp = prefs.getUInt("time", 0);
        prefs.end();
        return state;
    }

    void clear() {
        Preferences prefs;
        prefs.begin("storage", false);
        prefs.clear();
        prefs.end();
    }
};