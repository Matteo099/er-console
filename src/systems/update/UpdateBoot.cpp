#include <sdkconfig.h>

// Arduino otherwise confirms pending OTA images inside initArduino(), before
// setup(), the display and our services have been tested. The symbol is C.
#ifdef CONFIG_APP_ROLLBACK_ENABLE
extern "C" bool verifyRollbackLater() { return true; }
#endif
