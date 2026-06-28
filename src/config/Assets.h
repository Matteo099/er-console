#pragma once

#define CORE_DIR "/.core"
#define GAMES_DIR "/games"

#define AUDIO_DIR CORE_DIR "/audio"
#define CONFIG_DIR CORE_DIR "/config"
#define PORTAL_DIR CORE_DIR "/portal"
#define IMAGES_DIR CORE_DIR "/images"

// Configuration files
#define DEVICE_CONFIG_FILE CONFIG_DIR "/config.json"
#define WIFI_CFG_FILE CONFIG_DIR "/wifi.cfg"

// Portal files
#define PORTAL_INDEX_FILE PORTAL_DIR "/index.html"

// Audio files
#define AUDIO_STARTUP_FILE AUDIO_DIR "/stage.wav"
#define AUDIO_CLICK_FILE AUDIO_DIR "/fireball.wav"
#define AUDIO_CLICK1_FILE AUDIO_DIR "/click1.wav"
#define AUDIO_CLICK2_FILE AUDIO_DIR "/click2.wav"
#define AUDIO_GAME_OVER_FILE AUDIO_DIR "/lose.wav"
#define AUDIO_VICTORY_FILE AUDIO_DIR "/win.wav"
#define AUDIO_TICKING_CLOCK_FILE AUDIO_DIR "/tickingclock.wav"


// Image files
#define IMG_LOGO_FILE IMAGES_DIR "/logo.png"
#define IMG_BRAND_FILE IMAGES_DIR "/brand.png"
#define IMG_BRAND_NEGATIVE_FILE IMAGES_DIR "/brand_negative.png"