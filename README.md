# EscapeRoom Console

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