# Diagnostica microSD

## Stato finale della sessione

Firmware completo compilato, caricato su COM3 e verificato su due avvii
consecutivi: scansione completa di `/.core` e `/games` (incluso `1-lab`), otto
letture complete delle immagini per avvio con checksum attesi, ingresso nel
menu principale e nessun errore SD nei log. Acquisizioni:
`.pio/sd-stack-boot.log` e `.pio/sd-stack-repeat-boot.log`.

Configurazione verificata: Wi-Fi/portale spenti, SD prima del display,
diagnostica letture attiva, SPI SD a 400 kHz, task storage con 8192 byte di
stack e log per directory. Il margine minimo misurato e' 5172 byte: non prova
che i precedenti 4096 byte fossero esauriti. Il cambiamento di stack e i log
modificano anche il timing; non si attribuisce loro una causa unica dimostrata.
Questa configurazione e' diagnostica, con boot e trasferimenti piu' lenti.

La riaccensione completa ha ripristinato nel test minimale letture corrette
anche a 1 e 4 MHz. Il guasto e' quindi sensibile allo stato/reset e al percorso
di esecuzione, ma il meccanismo dell'intermittenza resta da dimostrare. Non
sono stati applicati wrapper SPI sperimentali o modifiche al core installato.

## Risultati dell'analisi

Il log fornito fallisce durante `SD.begin()` / `f_mount()`, prima del caricamento
dei file applicativi: `cmd: 0x11` e' CMD17, lettura di un singolo blocco. Il
driver non riceve una risposta al comando. I successivi errori `File system is
not mounted` sono una conseguenza, non una prova di PNG o percorsi errati.
Wi-Fi e portale risultavano gia' disabilitati nella build debug del log;
inoltre vengono inizializzati dopo questo errore.

Configurazione effettiva, verificata anche nelle sorgenti locali di
Arduino-ESP32 2.0.17 e LovyanGFX 1.2.19:

| Dispositivo | Host hardware | SCK | MOSI | MISO | CS | Clock configurato |
| --- | --- | --- | --- | --- | --- | --- |
| TFT ILI9341 | SPI2 | 12 | 11 | 13 | 10 | scrittura 20 MHz, lettura 10 MHz |
| Touch XPT2046 | SPI2 | 12 | 11 | 13 | 7 | 1 MHz |
| microSD | SPI3 (`SPIClass(HSPI)`) | 21 | 47 | 20 | 5 | 400 kHz anche dopo inizializzazione |

SD e display **non condividono l'host SPI** in questa configurazione. Il
`bus_shared=true` di LovyanGFX riguarda TFT e touch. Su ESP32-S3 i valori Arduino
`FSPI`/`HSPI` non vanno interpretati come i nomi storici dell'ESP32 classico.
La separazione si vede anche nelle
[sorgenti Espressif della versione usata](https://github.com/espressif/arduino-esp32/blob/2.0.17/cores/esp32/esp32-hal-spi.c).

`include/User_Setup.h` e i suoi clock TFT_eSPI non sono la configurazione del
display in esecuzione: il progetto utilizza i configuratori LovyanGFX sotto
`src/drivers/display/`. L'esempio `src/main_stress_test.cpp`, escluso dalla build
normale, usa anch'esso il bus SD di `SPIBusManager` e gli stessi pin. Inizializza
TFT e LVGL prima del mount SD, ma avvia il task di rendering solo dopo il mount.
Nel boot normale, invece, il task UI esegue gia' rendering e polling del touch
mentre il task storage monta la scheda. Questa e' una differenza concreta da
isolare, pur usando host distinti.

Non e' ancora dimostrata la causa dell'errore sul dispositivo. La prova con
componenti funzionanti rende utile confrontare prima ordine di inizializzazione,
host/pin effettivi e letture con rendering attivo. Aggiungere un mutex globale
tra i due host non e' una correzione giustificata da questa configurazione.

Altri due dettagli da tenere presenti quando si attivano gli ingressi: i pin 21
e 47 sono ancora esposti dagli alias `GPIO_FREE_5` / `GPIO_FREE_11`, ma sono
occupati dalla SD. Inoltre `MFRC522Driver` conserva il bus passato al costruttore,
mentre la libreria MFRC522 usata accede al globale `SPI` (SPI2), non al bus SD
passato al driver. Nessuno dei due punti spiega il primo mount del log, dove
gli ingressi vengono inizializzati dopo il fallimento; diventano rilevanti
quando si abilitano periferiche aggiuntive o script Lua.

## Opzioni in Console.h

- `WIFI_ENABLED=0`: non inizializza la radio, non carica le credenziali dalla SD,
  rifiuta le richieste di connessione e termina il task dopo l'inizializzazione.
- `PORTAL_ENABLED=0`: non avvia AP, DNS, server HTTP o letture del portale dalla
  SD; termina il task dopo l'inizializzazione. Richiede anche `WIFI_ENABLED=1`
  per essere attivo.
- `SD_READ_DIAGNOSTICS_ENABLED=1`: dopo un mount riuscito legge integralmente
  `/.core/images/brand_negative.png` e `/.core/images/logo.png` due volte,
  registrando byte letti/dimensione e checksum FNV-1a. Non scrive sulla scheda.
  Questa e' una verifica di completezza e ripetibilita': per verificare anche la
  correttezza dei dati, confrontare dimensioni/checksum con quelli dei file su PC.
  La scansione delle frequenze e degli host e' disponibile separatamente
  nell'ambiente `esp32-s3-devkitc-1-n16r8v-sd-probe`.
- `SD_INIT_BEFORE_DISPLAY=1`: monta e verifica SD prima di inizializzare TFT,
  touch e LVGL. Con diagnostica delle letture attiva ripete la verifica dopo
  l'inizializzazione della UI. Attualmente e' impostato a `1` per il confronto
  sul dispositivo; `0` ripristina l'ordine normale.

Le costruzioni degli oggetti `WebServer` / `WiFiServer` possono comparire nei
log verbose anche con rete disabilitata: non significano che AP o server siano
stati avviati.

## Confronto A/B

1. Lasciare `WIFI_ENABLED=0`, `PORTAL_ENABLED=0`, log su SD e recovery disabilitati.
   Impostare `SD_READ_DIAGNOSTICS_ENABLED=1` e `SD_INIT_BEFORE_DISPLAY=0`.
   Compilare, caricare e acquisire un avvio completo.
2. Impostare solo `SD_INIT_BEFORE_DISPLAY=1`, ricompilare e ripetere l'avvio.
   Questo confronto elimina dal primo mount sia l'inizializzazione del TFT sia
   il rendering e il polling del touch in parallelo. Se cambia il risultato,
   un ulteriore confronto con lo stress test permette di distinguere queste
   condizioni, perche' lo stress test inizializza TFT prima del mount ma avvia
   il rendering dopo.
3. Interpretare i risultati:

| Risultato | Indicazione |
| --- | --- |
| Mount fallisce anche prima del TFT | La UI e le sue transazioni non sono necessarie per riprodurre il problema; confrontare il percorso SPI/SD con l'esempio funzionante. |
| Letture corrette prima del TFT, errore dopo la sua inizializzazione | L'avvio del display/touch modifica una condizione rilevante; controllare pin/host effettivi e stato dei segnali. Non prova da solo un conflitto logico sul bus. |
| Funziona solo montando SD prima del TFT | Dipendenza dall'ordine o dal timing; non considerare il cambio d'ordine una soluzione definitiva senza verificare l'uso prolungato. |
| Mount riuscito ma byte incompleti o checksum diversi fra passaggi | Letture instabili riprodotte anche senza decoder PNG; indagare SPI/SD prima della UI. |
| Byte completi e checksum coerenti con PC, PNG ancora falliscono | Indagare decoder/formato PNG e memoria LVGL come problema successivo. |

Comandi (se `pio` non e' nel PATH, usare il terminale PlatformIO):

```powershell
pio run -e esp32-s3-devkitc-1-n16r8v-debug
pio run -e esp32-s3-devkitc-1-n16r8v-debug -t upload
pio device monitor -e esp32-s3-devkitc-1-n16r8v-debug -p COM3 -b 115200
```

Usare lo stesso ambiente per upload e monitor: l'exception decoder carica
l'ELF dell'ambiente selezionato, non quello dell'ultimo upload. Il progetto
imposta ora `default_envs` sull'ambiente debug anche per i comandi senza `-e`.
L'avviso di ELF mancante non causa il fallimento del mount SD.

Per calcolare su PC lo stesso checksum dei file campione, dalla root:

```python
from pathlib import Path
for name in ("brand_negative.png", "logo.png"):
    data = (Path("sd/.core/images") / name).read_bytes()
    checksum = 2166136261
    for byte in data:
        checksum = ((checksum ^ byte) * 16777619) & 0xffffffff
    print(name, len(data), f"{checksum:08x}")
```

Valori dei file attualmente inclusi nel repository (validi se sulla SD sono
state copiate queste stesse versioni):

| File | Byte | FNV-1a |
| --- | --- | --- |
| `brand_negative.png` | 10534 | `526cf1e2` |
| `logo.png` | 5057 | `61684d7f` |

GPIO20 e' anche USB D+ dell'ESP32-S3: se il connettore USB nativo e' collegato,
la linea puo' avere un secondo uso. Non serve cambiare il cablaggio per il test
A/B; usare il bridge USB-UART della scheda e mantenere le stesse condizioni
dell'esempio funzionante. Disabilitare USB CDC nel firmware non elimina una
connessione fisica su D+. Vedere la
[documentazione Espressif](https://docs.espressif.com/projects/esp-idf/en/v4.4.7/esp32s3/api-guides/usb-serial-jtag-console.html).

## Correzioni alla segnalazione degli errori

`EVT_STORAGE_READY` attualmente segnala che il task di inizializzazione e'
terminato, anche quando il mount fallisce. Non viene usato come prova del mount:
il caricamento delle immagini di boot e l'apertura LVGL controllano anche
`StorageSystem::isReady()`. Il boot puo' ancora proseguire senza SD.

Il callback LVGL di lettura confronta i byte attesi con quelli restituiti dal
file e segnala `LV_FS_RES_FS_ERR` sulle letture incomplete, conservando il
normale comportamento di EOF. Questo evita di mascherare un errore I/O come una
lettura riuscita; non ripara il montaggio della scheda.

Anche `readTextFile()` ora restituisce errore e scarta il testo parziale quando
la lettura si interrompe prima dell'EOF, invece di restituire successo.

## Prove sulla scheda del 2 novembre 2026

Upload e acquisizioni eseguiti direttamente su COM3 (bridge CH343), 115200 baud:

- Firmware completo a 1 MHz, SD prima di TFT/touch/LVGL: stesso errore CMD17.
  La UI non e' necessaria per riprodurre il problema.
- Firmware completo, mount prima del display a 400 kHz, 1 MHz e 4 MHz: prima
  scansione fallita su tutte le frequenze.
- Prima build minimale Arduino+SPI+SD: prima scansione fallita anche senza
  servizi OS, sia su SPI3 sia su SPI2, sempre con gli stessi pin.
- Build minimale estesa con lettura diretta del protocollo, ripetuta su due
  avvii: su SPI3 a 400 kHz mount e letture integrali delle due immagini riusciti,
  con dimensioni e checksum corrispondenti ai file del repository. A 1 e 4 MHz
  il mount puo' riuscire ma la successiva lettura RAW fallisce: il solo esito di
  `SD.begin()` non basta a certificare la lettura.
- Lettura SPI diretta del settore 0 su SPI3 a 400 kHz: firma `55aa` e CRC16
  ricevuto/calcolato `5693`, sia con CRC comandi attivo sia disattivo.
- Su SPI2, nel confronto sequenziale, CMD17 diretto restituisce R1=0 ma non
  arriva il token dati entro il timeout. Cambiare host non e' una soluzione
  dimostrata per questa scheda.
- Firmware completo a 400 kHz: mount e primi quattro controlli dei file riusciti,
  ma errori CMD17/CMD13 durante la successiva scansione `/games`, ancora prima
  di inizializzare il display, e ulteriori errori dopo la UI. Il clock ridotto
  migliora la lettura ma non costituisce da solo una soluzione verificata.
- Modifica sperimentale per inviare i comandi SD byte per byte: mount ancora
  fallito. La modifica e il relativo wrapper del linker sono stati rimossi.
- Nel successivo confronto diretto su SPI3, byte/blocco e clock dopo CS alto
  attivi/disattivi non hanno discriminato il guasto: i comandi di inizializzazione
  rispondono correttamente e CMD17 restituisce R1=0, ma manca il token dati.
  Questo mostra anche perche' i precedenti successi RAW non bastano a considerare
  stabile l'intero percorso di lettura.
- Dopo aver tolto alimentazione a ESP32 e microSD per almeno 10 secondi:
  lo stesso test minimale legge correttamente settore 0 e immagini a tutte e
  tre le frequenze (400 kHz, 1 MHz, 4 MHz), con i checksum attesi. Anche tutte
  le varianti RAW byte/blocco e clock CS alto attivi/disattivi passano con
  CRC16 `5693` e R1 entro 2 byte. Il log e' in
  `.pio/sd-after-power-cycle-probe.log`.
- Verifica settori FAT32 di `/games/1-lab`: MBR LBA 0, boot partizione LBA 8192,
  cluster giochi 6, cluster `1-lab` 10, primo settore directory LBA 25088 e
  relativo collegamento FAT LBA 9670 leggibili; fine catena `0fffffff`.
  Il test non dimostra una zona illeggibile o una catena corrotta in quella
  directory. Log: `.pio/sd-sector-probe-boot.log`.

Il cambiamento decisivo osservato e' la riaccensione, non il trasporto byte/blocco
o una specifica frequenza. Il risultato e' compatibile con uno stato della SD
che non viene ripristinato dal solo reset ESP32; non identifica da solo il
meccanismo che porta a quello stato. Un riavvio tramite RTS e un upload non
tolgono alimentazione alla microSD. In questa fase il clock operativo e' stato
mantenuto a 400 kHz; la successiva verifica a 4 MHz e' descritta sotto.

### Revisione UI e prestazioni

Il clock SD e' ora 4 MHz (`SD_SPI_CLOCK`); I2C resta a 400 kHz e non regola la
velocita' della SD. Con il firmware completo, display/touch e servizi attivi,
sono state misurate due letture integrali consecutive di ciascuna immagine:

| File | Byte | FNV-1a | Prima, 400 kHz | Ora, 4 MHz |
| --- | --- | --- | --- | --- |
| `brand_negative.png` | 10534 | `526cf1e2` | 326 ms | 26 ms |
| `logo.png` | 5057 | `61684d7f` | 209 ms | 13 ms |

La revisione evita inoltre il dump ricorsivo della scheda a ogni avvio,
i controlli ripetuti delle immagini nel percorso normale e le letture di PNG
per header e boot. La cache LVGL conserva le immagini decodificate; il monitor
mostra la capacita' fisica gia' nota, senza ricalcolare ogni frame lo spazio FAT.

Durante un primo test di cambi tema ripetuti sono ricomparsi errori CMD13
riscrivendo il documento di configurazione sulla SD. Questo limite delle
scritture non e' risolto dall'aumento del clock. Tema e suoni ora vengono
salvati in NVS, senza riscrivere la SD per ogni cambio. Il successivo test di
navigazione, temi, audio e immagini non ha prodotto errori di lettura SD;
anche il ripristino delle preferenze dopo un reset e' stato verificato.
Questo non certifica tutte le possibili operazioni di scrittura del portale
o degli script Lua. Log locali: `.pio/ui-previews/review-session.log` e
`.pio/ui-previews/persistence.log`.

La build finale, caricata su COM3 con la diagnostica UI disabilitata, monta
la SD a 567 ms ed entra nel menu a 1632 ms dall'avvio. Nei 12 secondi acquisiti
non risultano errori SD o decoder LVGL. Log: `.pio/ui-final-boot.log`;
build e upload: `.pio/ui-final-upload.log`. Il confronto precedente con dump
ricorsivo e controlli attivi arrivava al menu a circa 12 secondi: il guadagno
complessivo comprende anche la rimozione di quel lavoro diagnostico.

La configurazione normale mantiene `SD_INIT_BEFORE_DISPLAY=1` e disabilita
`SD_READ_DIAGNOSTICS_ENABLED`, `STORAGE_BOOT_LIST_ENABLED` e
`UI_DIAGNOSTICS_ENABLED`. Per comandi di ispezione e schermate vedere
[UI_THEME.md](UI_THEME.md).

### Riprodurre il test minimale

Il file `src/main_sd_probe.cpp` e' escluso dalle build normali. L'ambiente
dedicato compila solo quel file e le librerie Arduino SPI/SD/FS. Non avvia UI,
Wi-Fi, audio, LED animati o servizi del progetto, e non formatta/scrive la SD.
La versione attuale prova SPI3 sui pin SD, tre frequenze, file interi e letture
dirette CMD17 confrontando comandi byte/blocco e clock dopo CS alto. Se trova
un volume FAT32/512, legge anche i settori della directory `/games/1-lab` e il
relativo collegamento FAT, senza modificarli.
Per ripetere il confronto su SPI2, cambiare la chiamata finale a `checkHost()`
in `checkHost(FSPI, "SPI2/FSPI")`, mantenendo gli stessi pin.

```powershell
pio run -e esp32-s3-devkitc-1-n16r8v-sd-probe -t upload --upload-port COM3
pio device monitor -e esp32-s3-devkitc-1-n16r8v-sd-probe -p COM3 -b 115200
```

Per tornare al firmware completo:

```powershell
pio run -e esp32-s3-devkitc-1-n16r8v-debug -t upload --upload-port COM3
pio device monitor -e esp32-s3-devkitc-1-n16r8v-debug -p COM3 -b 115200
```

Acquisizioni locali: `.pio/sd-before-display-boot.log`,
`.pio/sd-frequency-boot.log`, `.pio/sd-standalone-boot.log`,
`.pio/sd-raw-boot.log`, `.pio/sd-raw-repeat-boot.log`.
Ulteriori acquisizioni: `.pio/sd-full-400k-boot.log`,
`.pio/sd-bytewise-boot.log`, `.pio/sd-sequence-boot.log`.
