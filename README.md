# EscapeRoom Console

## Diagnostica microSD / TFT / touch

Le opzioni di isolamento sono in [`src/config/Console.h`](src/config/Console.h):

```cpp
#define WIFI_ENABLED 0
#define PORTAL_ENABLED 0
#define SD_INIT_BEFORE_DISPLAY 1
#define SD_READ_DIAGNOSTICS_ENABLED 0
#define STORAGE_BOOT_LIST_ENABLED 0
#define UI_SCENE_ANIMATIONS_ENABLED 0
#define UI_DIAGNOSTICS_ENABLED 0
```

Wi-Fi e portale sono attualmente disabilitati per isolare i reset brownout
osservati con alimentazione USB del PC. Riportare entrambi i flag a `1` per
riattivarli; con Wi-Fi disabilitato anche il portale rimane spento.
Il precedente `ER_DISABLE_WIFI` della sola build debug
e' stato sostituito da queste opzioni.

La configurazione attuale inizializza SD prima del display. I controlli ripetuti
dei file e la scansione ricorsiva al boot sono opt-in, per evitare attese inutili.
Per ripristinare l'ordine di boot precedente impostare
`SD_INIT_BEFORE_DISPLAY=0`. L'ambiente predefinito PlatformIO e' quello debug,
cosi' upload e monitor senza selezione esplicita usano lo stesso ELF.

Per il confronto dei bus e la procedura A/B vedere
[`docs/SD_DIAGNOSTICS.md`](docs/SD_DIAGNOSTICS.md).

## Interfaccia TFT

La UI usa tre temi pastello, **Menta**, **Cielo** e **Corallo**, selezionabili da
Impostazioni > Tema. Le schede Audio, Lingua e Sistema centralizzano le altre
preferenze: musica ed effetti indipendenti, italiano/inglese, rete e monitoraggio.
Tema, lingua, audio e credenziali Wi-Fi vengono conservati nella memoria interna.
La rete resta configurabile anche con radio disabilitata; connessione e portale
rispettano i flag di avvio.

Configurazione, migrazione e traduzioni: [docs/SETTINGS.md](docs/SETTINGS.md).
Mixer a otto sorgenti, loop, volumi per file e conversione audio indipendente
da PlatformIO: [docs/AUDIO.md](docs/AUDIO.md).
Precaricamento di countdown, sconfitta e vittoria:
[docs/AUDIO_PRELOAD.md](docs/AUDIO_PRELOAD.md).
Scelta delle reti e inserimento password: [docs/NETWORK_UI.md](docs/NETWORK_UI.md).
Traduzioni dei giochi e aggiornamento dei file sulla microSD:
[docs/GAME_LOCALIZATION.md](docs/GAME_LOCALIZATION.md).

Architettura, palette, configurazione e acquisizione delle schermate:
[`docs/UI_THEME.md`](docs/UI_THEME.md).

Ottimizzazioni dei cambi scena e misure:
[`docs/UI_PERFORMANCE.md`](docs/UI_PERFORMANCE.md).
Protezione dei pulsanti durante lo scroll e test degli input:
[`docs/UI_SCROLL.md`](docs/UI_SCROLL.md).
Il font dei titoli e il generatore sono descritti in
[`assets/fonts/README.md`](assets/fonts/README.md).

# 1️⃣ Stack software – visione d’insieme

Il tuo stack è corretto e lo formalizziamo così:

```
┌──────────────────────────────────────────┐
│              LUA SCRIPTS                 │  ← Logica Escape Room
│ (scenari, enigmi, stati, puzzle, audio)  │
├──────────────────────────────────────────┤
│          LUA API / BINDINGS              │
│ (Display, Audio, Input, Storage, Timer)  │
├──────────────────────────────────────────┤
│        ESCAPE ROOM OS (C++)              │
│                                          │
│  - Boot Manager                          │
│  - Scene Manager (UI states)             │
│  - Device Manager                        │
│  - File System Manager                   │
│  - Audio Manager                         │
│  - Input Manager                         │
│  - Lua Runtime                           │
├──────────────────────────────────────────┤
│      ESP32 Framework (Arduino)           │
├──────────────────────────────────────────┤
│              ESP32-S3 HW                 │
└──────────────────────────────────────────┘
```

📌 **Concetto chiave**
Il firmware **non contiene la logica dell’escape room**.
Contiene solo:

* OS
* driver
* UI
* API Lua

Questo ti permette:

* cambiare escape room **senza ricompilare**
* caricare scenari da SD
* avere una UX “da computer”

---

# 2️⃣ Struttura del progetto PlatformIO

Questa è una struttura **pulita, scalabile e realistica**:

```
/src
 ├── main.cpp                ← entry point
 ├── os/
 │    ├── EscapeOS.h
 │    ├── EscapeOS.cpp
 │    ├── BootManager.h
 │    ├── BootManager.cpp
 │    ├── SceneManager.h
 │    ├── SceneManager.cpp
 │    └── TaskScheduler.h
 │
 ├── devices/
 │    ├── Display/
 │    │     ├── Display.h
 │    │     ├── TFTDisplay.cpp
 │    │     └── TouchController.cpp
 │    │
 │    ├── Audio/
 │    │     ├── AudioManager.h
 │    │     └── AudioManager.cpp
 │    │
 │    ├── Storage/
 │    │     ├── FileSystem.h
 │    │     └── SDFileSystem.cpp
 │    │
 │    └── Input/
 │          ├── InputManager.h
 │          └── Button.cpp
 │
 ├── ui/
 │    ├── Scene.h
 │    ├── BootScene.cpp
 │    ├── LoadingScene.cpp
 │    └── MainMenuScene.cpp
 │
 ├── log/
 │    ├── Logger.h
 │    └── Logger.cpp
 |
 ├── lua/
 │    ├── LuaVM.h
 │    ├── LuaVM.cpp
 │    └── LuaBindings.cpp
 │
 └── config/
      └── BoardConfig.h
```

📌 **Perché così:**

* `os/` → cervello del sistema
* `devices/` → hardware isolato
* `ui/` → UX / scene / stati visuali
* `log/` → logging
* `lua/` → mondo dinamico
* `config/` → pin, parametri, feature



Prossimi step:
1️⃣ Boot visuale (BootScene + progress bar su TFT)
2️⃣ Device abstraction reale (Display / SD / Audio)
3️⃣ Event / Queue per comunicare boot progress alla UI
4️⃣ Fallback / error handling nel boot (safe mode)

1️⃣ Implementare Display driver reale (TFT 2.8 + touch)
2️⃣ Transizione Boot → Loading → Main Menu
3️⃣ Error handling visuale (boot fail → safe mode)
4️⃣ Persistenza log su SD (ring buffer)

## compilare

edit platform.ini:

``` ini
build_flags =
    -Isrc
    -Iinclude
```

run commands:

```
cp include\lv_conf.h .pio\libdeps\esp32-s3-devkitc-1-n16r8v
cp include\User_Setup.h .pio\libdeps\esp32-s3-devkitc-1-n16r8v\TFT_eSPI\User_Setup.h
```


## Esempio di utilizzo di LVGL

https://randomnerdtutorials.com/lvgl-cheap-yellow-display-esp32-2432s028r/

## Lua

https://github.dev/Sasszem/ESP8266-Arduino-Lua

## Captive portal

The network screen now focuses on connection status and QR onboarding for the ESP32 access point.
All device configuration is stored in `/.core/config.json` on the SD card, and the portal is served from `sd/.core/portal`.

Frontend source lives under `frontend/` as a Vue 3 + Vuetify SPA.
Build it with:

```bash
cd frontend
npm install
npm run build
```

The portal API exposes status, configuration, file browsing, upload, delete, and reboot endpoints over REST.
