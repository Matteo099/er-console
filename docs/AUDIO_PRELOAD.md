# Precaricamento dei suoni di partita

La partita prepara `tickingclock.wav`, `lose.wav` e `win.wav` prima di mostrare
la schermata pronta per Avvia. Il costo della lettura SD viene anticipato alla
fase di preparazione; gli eventi di gioco riproducono poi i dati dalla PSRAM.
Non vengono cambiati formato, contenuto o nome dei file audio esistenti.

## Flusso Scene / View

`GameScene::onEnter()` crea le SubView e mostra il testo localizzato
"Preparazione dei suoni...". Dopo il primo rendering, `GameAudioAssets` richiede
tre preload al task audio. La Scene continua ad aggiornarsi mentre attende;
il timer non parte e il pulsante Avvia del gioco non e' ancora disponibile.
Quando i caricamenti sono terminati, vengono inizializzati dispositivi e Lua,
poi appare la schermata pronta. Il caricamento delle SubView non riproduce suoni.

- `MainSubview::setRunning(true)` richiede subito il loop quando il tempo e'
  compreso fra 1 e 60 secondi. Non attende il successivo aggiornamento o il
  precedente intervallo di 500 ms. Questo intervallo rimane solo per ritentare
  una richiesta rifiutata. Anche il passaggio da 61 a 60 secondi richiede subito
  l'audio. Pausa, scadenza e mute fermano solo il ticchettio posseduto dalla View.
- `FailSubview::show()` e `VictorySubview::show()` rendono visibile la SubView e
  richiedono una sola riproduzione dell'effetto. Richiamare show su una SubView
  gia' visibile non avvia un secondo suono.
- `GameScene::completeGame()` ferma i suoni precedenti prima di mostrare Victory;
  la riproduzione appartiene alla SubView. E' stata eliminata la seconda
  richiesta precedentemente emessa dalla Scene.

La musica del menu viene fermata entrando nella partita, anche se sospesa da
Musica OFF: in questo modo non trattiene spazio nella cache. Al ritorno al menu
il suo ID viene verificato e il brano riparte, se necessario, dopo il cleanup
della schermata precedente. Non viene duplicato passando fra le pagine del menu.

## Cache e concorrenza

`AudioSystem::retainAsset(path)` restituisce un ID di prenotazione, oppure zero
se percorso, tabella o coda non consentono la richiesta. Il chiamante consulta
`assetState(id)`: `Pending`, `Ready`, `Failed` o `Missing`.
Il task audio pubblica Ready solo dopo lettura completa e validazione del WAV.
Richieste dello stesso percorso condividono il caricamento e un contatore di
riferimenti. `releaseAsset(id)` libera la propria prenotazione; l'ultimo
rilascio consente nuovamente l'espulsione del file inattivo dalla cache.

Una prenotazione protegge i dati anche quando nessuna sorgente li sta suonando.
Le generazioni degli ID impediscono a completamenti o rilasci tardivi di agire
su una nuova prenotazione che riutilizza lo slot. I preload ancora in coda
vengono ignorati se la Scene ha gia' rilasciato la propria prenotazione.
Il caricamento eventualmente gia' in corso puo' terminare e lasciare dati
inattivi riutilizzabili nella cache.

`GameAudioAssets` mantiene le tre prenotazioni per tutta la Scene, incluse
sconfitta e vittoria, e le rilascia in uscita. Riprova una coda piena e usa
un'attesa massima di 15 secondi. File mancanti, non validi o non caricabili
vengono segnalati nei log, ma l'audio opzionale non impedisce la partita.
In tali casi non si puo' garantire l'avvio rapido del suono mancante.

Le letture restano nel task audio e acquisiscono il lock SPI per blocchi di
1024 byte, lasciando eseguire display, touch e altri lettori SD fra i blocchi.
Ogni 32 KiB il task rilascia la CPU per un tick: i caricamenti consecutivi non
devono impedire al task idle del core 0 di alimentare il watchdog.

Il budget totale resta **4 MiB** e otto file in PSRAM. Le prenotazioni non
incrementano questo limite e i file attivi restano a loro volta protetti.
I tre WAV attuali occupano 2794986 byte: restano circa 1,33 MiB per altri file
unici durante una partita. Se non basta, la nuova richiesta fallisce senza
espellere i suoni prenotati o in uso. La conversione al formato canonico con
`scripts/standardize_audio.py` riduce il costo degli originali stereo/48 kHz;
gli output vanno copiati separatamente sulla microSD.

## Prove e limiti delle misure

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`:

```powershell
python scripts/audio_preload.py
python scripts/audio_preload_game.py
```

Il primo comando verifica prenotazioni, condivisione, rilasci, ID riutilizzati,
coda piena, timeout e file protetti. Mostra poi una GameView senza eseguire Lua
o operare periferiche di gioco, carica gli asset reali, introduce pressione
nella cache e prova countdown, pausa/ripresa, Fail e Victory.
Il secondo usa il vero Test - Ciclo da un minuto, i cui input sono disattivati,
e ne verifica l'avvio e la scadenza naturale attraverso Scene/View e Lua.
Entrambi usano un master temporaneo all'1% per la riproduzione di prova.
Non modificano le preferenze persistenti; gli effetti devono essere gia' ON.

Il log `WAV started: ... cache=hit delay=N ms` misura il tempo dalla richiesta
all'inserimento della sorgente nel mixer. Non misura il tempo acustico al
microfono: si aggiungono il buffer I2S e l'introduzione contenuta nel WAV.
Negli originali PC il primo campione significativo e' a circa 23 ms per clock
e sconfitta e 50 ms per vittoria. Queste introduzioni musicali sono conservate.

Verifiche del 7 novembre 2026:

- 19 controlli di prenotazione e ciclo di vita superati sull'ESP32.
- Precaricamento a freddo dei tre originali: circa 7,3 secondi, durante la
  preparazione; successivi avvii dalla GameView in 0-1 ms, tutti cache hit e
  senza apertura dei corrispondenti file SD, anche dopo pressione sulla cache.
- Partita reale Test - Ciclo: avvio del countdown in 0 ms e sconfitta naturale
  dopo un minuto in 1 ms, entrambi cache hit; preferenze iniziali conservate.
- La prima prova ha rilevato un watchdog IDLE0 durante letture consecutive;
  la pausa periodica nel lettore ha eliminato il problema nelle prove ripetute.
- Regressioni: 25 verifiche del feedback volume, 41 del mixer e 30 generali
  superate; 139 chiavi di traduzione verificate in entrambe le lingue.
- Build finale caricata su COM3 con diagnostica disattivata, Wi-Fi `1` e
  portale `0`. Avvio acquisito per 22 secondi: menu a 1761 ms, senza errori SD,
  panic o riavvii durante l'acquisizione.

Log: `.pio/audio-preload-check/` (prima prova),
`.pio/audio-preload-yield-check/` e `.pio/audio-preload-game/` (prove riuscite).
Regressioni in `.pio/audio-preload-regression/`; upload e boot in
`.pio/preload-final-upload.log` e `.pio/preload-final-boot.log`.
ELF/BIN corrispondenti: `.pio/preload-symbols/final.*`.
