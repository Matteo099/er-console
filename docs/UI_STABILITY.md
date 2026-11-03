# Panic dopo la navigazione e i salvataggi delle preferenze

## Causa e riproduzione

Il panic del 3 novembre 2026 e' `Cache disabled but cached memory region
accessed`. `MonitorScene::onEnter()` chiama `CpuMonitor::begin()`, che registra
callback del tick FreeRTOS su entrambi i core. I callback restano registrati
anche uscendo da Risorse: e' sufficiente aver aperto quella schermata una volta.

I tick possono essere eseguiti anche durante le operazioni sulla flash interna,
quando la cache e' disabilitata. Prima della correzione, `tickHook0()` e
`tickHook1()` risiedevano in `.flash.text`: il processore tentava quindi di
eseguire codice non accessibile. Un salvataggio delle preferenze NVS espone
il difetto; il fatto che l'ultimo messaggio seriale riguardi un suono non
attribuisce il panic al driver audio.

Riferimenti della versione ESP-IDF usata dal progetto:
[implementazione dei tick hook](https://github.com/espressif/esp-idf/blob/v4.4.7/components/esp_system/freertos_hooks.c)
e [vincoli IRAM](https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32s3/api-reference/system/intr_alloc.html#iram-safe-interrupt-handlers).

L'ELF originale del log utente non era piu' presente. La ricostruzione dello
stesso codice assegna `0x420085aa` a `tickHook0()`, con gli stessi indirizzi
FreeRTOS/IPC nel resto dello stack. Per evitare di basare la diagnosi soltanto
su questa ricostruzione, il difetto e' stato riprodotto sulla scheda con la
diagnostica UI attiva e un ELF conservato:

- Log: `.pio/crash-repro-repeat/session.log`.
- ELF: `.pio/crash-symbols/repro.elf`, SHA256 che inizia con
  `94c8ab8fcca1f3b1`, corrispondente al panic acquisito.
- PC della riproduzione: `0x420085f2`, `tickHook0()`, `CpuMonitor.cpp:31`
  prima della correzione.
- Stack: `tickHook0` -> `xTaskIncrementTick` -> `xPortSysTickHandler` ->
  `SysTickIsrHandler` -> `_xt_lowint1` -> `spi_flash_op_block_func` -> `ipc_task`.

## Correzione

`src/core/CpuMonitor.cpp` usa `IRAM_ATTR` per le quattro callback e `DRAM_ATTR`
per i contatori. Le callback incrementano soltanto contatori: non allocano,
non stampano e non chiamano codice in flash. Il controllo dei simboli del
firmware corretto conferma `.iram0.text` per le funzioni e `.dram0.data` per
i contatori. Gli stili UI, NVS e i bus restano operativi.

## Test sulla scheda

La build corretta ha completato **20 cicli, 61 salvataggi NVS e 206,9 secondi
senza panic o riavvii**. Nel medesimo intervallo non sono stati rilevati
`Card Failed`; nelle acquisizioni precedenti gli errori SD erano intermittenti,
quindi questo esito non ne dimostra la risoluzione. Log completo:
`.pio/crash-fixed-stress/session.log`. ELF diagnostico conservato:
`.pio/crash-symbols/fixed-diagnostic.elf`, SHA256 `a306eda5ad7c1683...`.

La build finale esclude i comandi diagnostici e conserva la correzione CPU.
ELF e BIN sono archiviati in `.pio/crash-symbols/final.*`, con gli hash delle
build di confronto in `.pio/crash-symbols/sha256.json`.
Upload finale riuscito (`.pio/crash-final-upload.log`), ELF SHA256
`5218559c2996becf...`. Nel successivo avvio di 10 secondi la SD risulta pronta
a 615 ms e il menu a 1688 ms, senza panic o errori SD:
`.pio/crash-final-boot.log`. Questi tempi descrivono quel singolo avvio.

Con `UI_DIAGNOSTICS_ENABLED=1` nell'ambiente debug e monitor seriale chiuso:

```powershell
pio run -e esp32-s3-devkitc-1-n16r8v-debug -t upload --upload-port COM3 --disable-auto-clean
& "$env:USERPROFILE/.platformio/penv/Scripts/python.exe" scripts/ui_stability.py --port COM3 --cycles 20
```

Il test apre prima Risorse, poi usa gli eventi reali dei pulsanti per visitare
Info, Home, Impostazioni e Risorse, alternare i tre temi e attivare/disattivare
i suoni. Attende il salvataggio NVS dopo ogni modifica e scorre il monitor.
Conserva il log completo, inclusi i registri in caso di panic. A fine prova
riuscita ripristina tema e suoni iniziali. Non avvia giochi o script Lua.

Un esito senza riavvii non certifica le letture SD: il riepilogo conta
separatamente i messaggi `Card Failed`. Per gli errori SD vedere
[SD_DIAGNOSTICS.md](SD_DIAGNOSTICS.md). Riportare il flag diagnostico a `0`
e caricare nuovamente al termine delle verifiche.

## Simboli per il decoder

L'avviso `firmware.elf does not exist` significa che il monitor non dispone
dei simboli per decodificare il backtrace. Prima dell'upload compilare nello
stesso ambiente del monitor e conservare insieme ELF e BIN. Per le prove
di questo intervento le copie sono in `.pio/crash-symbols`, fuori dalla
directory di build che PlatformIO puo' ripulire.

```powershell
pio run -e esp32-s3-devkitc-1-n16r8v-debug -t upload --upload-port COM3 --disable-auto-clean
pio device monitor -e esp32-s3-devkitc-1-n16r8v-debug -p COM3 -b 115200
```

Per un vecchio panic usare l'ELF della build caricata in quel momento e
confrontare l'hash riportato dal dispositivo. Ricompilare una revisione diversa
puo' spostare gli indirizzi e non garantisce una decodifica corretta.
