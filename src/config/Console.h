#pragma once

#define LOG_STORAGE_ENABLED 0
#define RECOVERY_STORAGE_SYSTEM_ENABLED 0

// Set to 1 to restore networking after SD diagnostics.
#ifndef WIFI_ENABLED
#define WIFI_ENABLED 0
#endif
#ifndef PORTAL_ENABLED
#define PORTAL_ENABLED 0
#endif
// Disabling Wi-Fi also disables the portal, including its access point.

// Optional A/B test: mount SD before TFT/touch initialization.
#ifndef SD_INIT_BEFORE_DISPLAY
#define SD_INIT_BEFORE_DISPLAY 1
#endif

// Read boot images twice and compare byte counts/checksums; no writes to SD.
#ifndef SD_READ_DIAGNOSTICS_ENABLED
#define SD_READ_DIAGNOSTICS_ENABLED 0
#endif

// Expensive directory dumps are separate from normal boot.
#ifndef STORAGE_BOOT_LIST_ENABLED
#define STORAGE_BOOT_LIST_ENABLED 0
#endif

// Development-only serial UI inspection/navigation.
// Full-screen slides/fades are expensive on the 20 MHz SPI display.
#ifndef UI_SCENE_ANIMATIONS_ENABLED
#define UI_SCENE_ANIMATIONS_ENABLED 0
#endif

#ifndef UI_DIAGNOSTICS_ENABLED
#define UI_DIAGNOSTICS_ENABLED 0
#endif
