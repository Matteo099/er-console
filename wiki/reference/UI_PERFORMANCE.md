# Cambi scena e caricamenti

Le Scene e le View conservano i loro ruoli. I dati condivisi hanno una cache
con vita applicativa; le View continuano a essere create e distrutte a ogni
navigazione. Non vengono conservate scene con eventi o puntatori scaduti.

## Lavoro spostato fuori dall'apertura delle viste

- Impostazioni usa `DeviceConfigRepository::loadCached`: configurazione gia'
  letta dai servizi e preferenze NVS. Aprire la schermata non rilegge la SD.
  Salvataggi dal portale e letture esplicite aggiornano la cache. Un errore
  successivo non cancella l'ultima configurazione valida.
- `GameCatalog` precarica i metadati sulla Home con un task a bassa priorita'
  sul core 0. Il task usa copie dei dati e non accede a LVGL o alle Scene.
  Il catalogo si apre subito: usa i dati pronti oppure mostra Caricamento giochi,
  lasciando disponibile il ritorno al menu. La freccia circolare aggiorna i
  metadati dopo modifiche ai file. Non vengono precaricati o eseguiti gli script Lua.
  Se non riesce a leggere alcun gioco, fa un solo secondo tentativo prima di
  mostrare l'errore; non entra in un ciclo di tentativi continuo.
- Un aggiornamento fallito conserva il catalogo precedente se non produce dati
  utilizzabili. Le nuove informazioni arrivano alle View dal task UI.
  Il gioco selezionato viene copiato prima di richiedere la scena successiva,
  cosi' un aggiornamento in background non invalida la selezione.
- Capacita' della scheda e scansione dello spazio occupato vengono lette
  tramite cache: la telemetria UI non attende il mutex SPI per la capacita'.
- Se il tema scelto e' gia' attivo, non viene riapplicato all'intero albero LVGL.

## Rendering

Il TFT resta a **20 MHz**. Trasmettere 320x240 pixel RGB565 richiede da solo
circa 61,4 ms, senza comandi SPI o rendering: aumentare soltanto la frequenza
dei timer LVGL non rende possibili animazioni a 60 fps.

`UI_SCENE_ANIMATIONS_ENABLED=0` in `Console.h` usa sostituzioni immediate,
senza muovere o fondere due schermate complete. L'opzione a `1` conserva le
transizioni animate richieste dalle Scene, per confronti o display piu' veloci.

Il percorso LovyanGFX usa due buffer RGB565 interni e allineati, ciascuno da
320x40 pixel (25.600 byte). LVGL disegna il prossimo blocco mentre il DMA
trasmette quello precedente. Il callback di attesa rilascia il buffer solo
dopo `waitDMA` e `endWrite`; prima di leggere il touch viene chiuso un eventuale
trasferimento pendente sullo stesso SPI2. La frequenza SD resta invariata.
Il driver scambia i byte RGB565 nel buffer e usa `swap565_t`, cosi' LovyanGFX
puo' inviare direttamente quel buffer: con `rgb565_t` eseguiva ancora copie
e conversioni in blocchi interni prima del trasferimento.

`LV_OBJ_STYLE_CACHE=1` evita di ricalcolare continuamente i gruppi di proprieta'
di stile. I font Escaperiom sono bitmap non compressi inclusi nel firmware:
non richiedono accessi SD o rasterizzazione vettoriale sul dispositivo.
Vedere [la configurazione display LVGL 9.4](https://lvgl.io/docs/open/9.4/details/main-modules/display/setup.html)
e il [convertitore ufficiale](https://github.com/lvgl/lv_font_conv).

## Diagnostica

Con `UI_DIAGNOSTICS_ENABLED=1`, `ui perf reset` azzera i contatori;
`ui perf` stampa tempi di preparazione della scena e del ciclo di refresh LVGL
in microsecondi. I tempi di refresh non sono una misura di sincronizzazione
del pannello o una certificazione visiva del segnale SPI.
`ui quiet on` silenzia l'audio solo per la sessione; `ui quiet off` ripristina
la preferenza salvata. I comandi non salvano queste modifiche in NVS.

Per ripetere le misure con il monitor PlatformIO chiuso:

```powershell
python scripts/diagnostics/ui_performance.py --port COM3 --quiet --output .pio/ui-performance
```

## Misure del 4 novembre 2026

Sulla stessa ESP32-S3 e sullo stesso TFT 320x240 sono state acquisite:

| Misura | Prima | Versione ottimizzata |
| --- | --- | --- |
| Preparazione Impostazioni | 84–1302 ms | 51–52 ms |
| Preparazione catalogo | 712–1226 ms | circa 38 ms con catalogo popolato |
| Massimo refresh Impostazioni | 162–163 ms | circa 112 ms |
| Massimo refresh Info | 121–122 ms | circa 87 ms |

Log e dati: `.pio/scene-perf-baseline`, `.pio/scene-perf-direct-dma` e
`.pio/scene-perf-final` (ultimo passaggio con catalogo popolato).
Il confronto complessivo e' indicativo: la prima sessione aveva Wi-Fi/portale
attivi, poi l'utente ha autorizzato lo spegnimento della radio dopo nuovi reset
`BROWNOUT_RST`, osservati anche a volume zero. La configurazione finale resta
quindi `WIFI_ENABLED=0`, `PORTAL_ENABLED=0`. Non attribuire l'intero miglioramento
alle sole modifiche software. Nei primi passaggi il catalogo preparava la vista
di attesa/errore in circa 54 ms; nell'ultimo, con i dati pronti, ha impiegato
37,8 ms di preparazione e circa 94 ms nel massimo ciclo di refresh.

Il confronto del solo percorso DMA e' invece avvenuto con la stessa
configurazione a radio spenta, nuovi font e cache gia' attive: il massimo
refresh del catalogo e' passato da circa 100 a 77 ms e quello di Impostazioni
da circa 140 a 112 ms. Dati intermedi: `.pio/scene-perf-optimized`.
La sessione `.pio/scene-perf-direct-dma/session.log` non contiene reset,
panic o `Card Failed`; questo non dimostra la stabilita' generale della SD.

La verifica `.pio/font-game-final/session.log` ha selezionato Test - Loop dalle
etichette effettive della UI, caricato lo script con 60 secondi, avviato e
abbandonato tramite conferma. Ha inoltre cambiato i tre temi e salvato quattro
volte le preferenze NVS con il monitor CPU attivo, senza panic o reset.
Sono presenti due `Card Failed`: la causa degli errori SD resta da indagare.
Le immagini in `.pio/font-game-final` mostrano titoli, timer, popup e sottoviste;
quelle in `.pio/font-ui-review` mostrano Home nei tre temi, Impostazioni e Risorse.
Gli header riservano una sola riga con ellissi per non far uscire titoli lunghi
dalla fascia superiore. Le immagini sono acquisizioni del rendering LVGL.

Upload finale riuscito: `.pio/ui-font-final-upload.log`. I simboli e il binario
sono conservati in `.pio/ui-font-symbols/final.*`, con gli hash in
`final-sha256.json` (ELF `2c74c444674b1af7...`). Il controllo finale di avvio
di 32 secondi ha raggiunto la Home a 1645 ms e precaricato i cinque giochi
entro 3560 ms, senza panic o reset. Sono presenti tre `Card Failed` prima
del recupero della lettura: `.pio/ui-font-final-boot.log`.
La porta COM3 e' stata rilasciata. Il firmware finale ha diagnostica e
animazioni di cambio scena disabilitate, oltre a Wi-Fi e portale.

I caricamenti effettivi di script Lua, immagini non in cache o file mancanti
possono ancora richiedere attese. La cache non corregge gli errori elettrici
o di comunicazione della SD. Anche il portale e le scansioni storage possono
competere fra loro per l'accesso alla scheda.
