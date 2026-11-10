# Interfaccia TFT e temi

La struttura resta `Scene` -> `View` -> componenti e `SubView`. Le Scene
gestiscono servizi, eventi e preferenze; le View creano oggetti LVGL e inviano
`ButtonClickedEvent`. Il tema non accede alla SD o ai servizi.

## Palette e componenti

`src/systems/ui/UITheme.*` raccoglie i colori e gli stili condivisi. Per una
nuova schermata usare `Screen`, `SceneLayout`, `BrandHeader`, `Button` e `Label`.
Applicare `UITheme::card`, `secondary`, `muted` e `accent` agli elementi secondo
il loro ruolo. Gli stili condivisi aggiornano anche gli oggetti gia' esistenti
quando cambia il tema; evitare colori locali se devono seguire la palette.

| Nome | Valore configurazione | Sfondo | Accento | Testo principale |
| --- | --- | --- | --- | --- |
| Menta | `mint` | `#F3F8F5` | `#C8E7D8` | `#263D34` |
| Cielo | `sky` | `#F3F7FC` | `#D3E5FA` | `#293B52` |
| Corallo | `coral` | `#FFF6F3` | `#F5D5CC` | `#4B3532` |

La selezione usa testo, bordo e segno di spunta, oltre al colore. I pulsanti
hanno un'area di almeno 44 px. `SceneLayout` mantiene intestazione e azioni
visibili; il corpo usa flex, larghezze percentuali, testo a capo e scorrimento
verticale. Il target verificato e' il TFT 320x240 in landscape.

I font Montserrat usano i font Roboto Mono gia' presenti come fallback per
gli accenti Latin-1. Il timer mantiene il font numerico monospace grande.
Titoli e intestazioni usano `UITheme::heading`: Escaperiom Display a 22 px;
18 px per i capitoletti e 26 px per i titoli principali. Il font comprende
gli accenti italiani e usa forme maiuscole anche per le minuscole. Testi lunghi
e pulsanti conservano i font di lettura. Generatore, TTF/WOFF2 e rigenerazione
dei bitmap sono descritti in [assets/fonts/README.md](../assets/fonts/README.md).
La dipendenza dal descrittore privato del tema LVGL 9.4 e' isolata in
`UITheme.cpp`: verificarla prima di aggiornare la versione di LVGL.

Il titolo prodotto e' centralizzato in `src/config/Branding.h`: **Escaperiom**.
`EscaperiomLogo` riproduce il simbolo a labirinto e il logotipo geometrico di
`sd/.core/images/logo.svg`, direttamente nelle callback di disegno LVGL,
senza leggere immagini dalla SD. Simbolo, nucleo e testo seguono i colori
`strong`, `accent` e `ink` del tema. L'SVG usa la variante Menta su sfondo
trasparente; il portale usa le esportazioni `escaperiom*.svg` in
`frontend/src/assets/images`. Dopo una modifica al portale eseguire
`npm run build` in `frontend` e copiare `sd/.core/portal` sulla scheda fisica.

`SceneManager` gestisce un'unica `DeviceStatusBar` sul layer superiore LVGL,
presente in alto a destra in tutte
le schermate, anche sopra QR e conferme. Lo stato si aggiorna ogni 500 ms.
`BrandHeader` riserva lo spazio necessario al gruppo. I colori semantici restano
gli stessi nei tre temi: verde per disponibile, giallo per attesa/degrado,
rosso con una X sovrapposta all'icona per indisponibile/errore.

- SD: verde dopo il montaggio o una lettura completa riuscita; gialla `SD!`
  dopo un errore I/O osservato; rossa con X se non montata o dopo cinque
  errori senza una lettura completa riuscita. Una semplice apertura di file
  non azzera gli errori. La mancanza di un file opzionale (`ENOENT`/`ENOTDIR`)
  non viene classificata automaticamente come guasto della scheda.
  Un file obbligatorio del gioco che non si riesce a leggere segnala invece
  degrado, anche quando Arduino traduce l'errore sottostante in `ENOENT`.
- Wi-Fi: verde `OK` se connesso, verde `AP` per la rete locale del portale,
  giallo `...` durante la connessione, rosso con X `OFF` se disconnesso e
  rosso con X `!` in errore. `AP` non indica accesso a Internet.

Lo stato SD usa gli esiti osservati nei percorsi di testo, cache audio e
immagini LVGL; non effettua scansioni o letture periodiche dal thread UI.
Non e' un sensore di rimozione: senza una nuova operazione I/O lo stato
resta quello dell'ultima verifica. Un audio letto parzialmente non entra
in cache come file valido.
Anche il controllo periodico di `/.core` nel task storage conta un errore
se la directory non e' piu' leggibile; il solo `exists` riuscito non azzera
gli errori delle letture precedenti.

Con `WIFI_ENABLED=0` l'indicatore resta visibile come `OFF` con X.
La pagina Rete permette di preparare le credenziali, indicando che la radio
e' disabilitata; non avvia connessioni o access point.

Il QR contiene il nome dell'access point effettivamente avviato, senza
rileggere la configurazione SD; i caratteri riservati dell'SSID vengono
escapati. Nome prodotto e marchio non cambiano il nome della rete gia'
configurato sul dispositivo.

## Schermate e disponibilita'

- Home: stato microSD e Wi-Fi, accesso al catalogo, impostazioni e informazioni.
  Il pulsante Gioca e' disabilitato quando il filesystem non e' disponibile.
- Impostazioni: Tema, Audio, Lingua e Sistema; musica ed effetti hanno ciascuno
  interruttore e volume. Sistema raccoglie risorse e rete.
  Indietro da Risorse o Rete torna alla scheda Sistema.
- Risorse: spazio SD occupato, totale, libero e percentuale. La scansione FAT
  avviene in un task dedicato e il risultato viene conservato dal servizio
  storage; Aggiorna richiede una nuova lettura. Se fallisce compare uno stato
  esplicito, senza mostrare zero come spazio occupato.
- Rete: selezione SSID da elenco ordinato per segnale, barre RSSI e lucchetti
  aperto/chiuso. Password con occhio, Annulla e Conferma. Vedere
  [NETWORK_UI.md](NETWORK_UI.md). Accesso al portale e QR solo con entrambi i flag
  `WIFI_ENABLED` e `PORTAL_ENABLED` attivi e con il portale effettivamente avviato.
  Un servizio disabilitato non produce URL o QR dimostrativi.
  Il pulsante QR Wi-Fi resta nella barra inferiore; la finestra QR e' un
  overlay `LV_OBJ_FLAG_FLOATING`, escluso dal layout della schermata e dallo
  scorrimento. La cornice bianca conserva lo spazio libero attorno ai moduli.
- Risorse: aggiornamento ogni secondo, capacita' fisica SD gia' nota al driver
  (senza scansione dello spazio libero), RAM/PSRAM, CPU e uptime. La sezione
  alimentazione compare solo se il relativo sensore e' disponibile.
- Catalogo: stato vuoto, contatore, navigazione e pulsante Avvia. I controlli
  non utilizzabili vengono disabilitati.
- Gioco: timer, pausa, esito e oggetti condividono colori e controlli. Gli
  oggetti si nascondono quando assenti; le immagini mantengono le proporzioni
  nello spazio disponibile e i contenuti lunghi scorrono.

## Preferenze

`DeviceConfigRepository` combina i valori predefiniti, il documento su SD e
le preferenze locali NVS, in quest'ordine. I nomi precedenti `light`/`dark`
ricadono su Menta; `blue` e `red` sono alias di Cielo e Corallo.

Tema, lingua e audio vengono applicati subito e salvati dopo 400 ms di quiete
o all'uscita dalle impostazioni. Usano il namespace NVS `console-ui`, record
versionato `prefs-v2`: non riscrivono il JSON della SD per ogni tocco. Questo consente
di mantenere le preferenze anche con SD assente. Un salvataggio completo dal
portale aggiorna anche questi valori NVS; il caricamento dal repository
restituisce gli stessi valori effettivi a tutti i servizi.

## Prestazioni e diagnostica

`I2C_FREQUENCY=400000` riguarda sensori e piccoli display I2C.
`SD_SPI_CLOCK=4000000` riguarda la microSD SPI. Aumentare I2C non accelera la SD.
Il boot normale non esegue il dump ricorsivo della scheda o i controlli ripetuti
di tutti i byte delle immagini. Header e boot usano testo e grafica LVGL,
senza leggere PNG dalla SD. Il buffer display e' RGB565 con stride coerente;
la cache delle immagini decodificate e' 1 MiB con PSRAM, 128 KiB senza.

Per ispezionare l'interfaccia dalla seriale, impostare temporaneamente
`UI_DIAGNOSTICS_ENABLED=1` in `src/config/Console.h` e usare l'ambiente debug:

```powershell
pio run -e esp32-s3-devkitc-1-n16r8v-debug -t upload --upload-port COM3
& "$env:USERPROFILE/.platformio/penv/Scripts/python.exe" scripts/ui_capture.py --port COM3 --reset --command "ui settings" --tree --snapshot settings
```

Chiudere il monitor seriale prima di acquisire. Lo script usa pyserial
dell'ambiente PlatformIO e salva PNG reali RGB565 e log in `.pio/ui-previews`.
Non richiede Pillow. Sono disponibili `ui home`, `ui settings`, `ui games`,
`ui monitor`, `ui about`, `ui theme mint|sky|coral`, `ui click X Y`, `ui tree`
e `ui sdcheck`. Quest'ultimo confronta due letture integrali per immagine,
dimensioni, tempi e checksum FNV-1a. Le anteprime `ui game|victory|fail|objects`
mostrano le sole View senza avviare Lua o comandare gli attuatori.

Con Wi-Fi abilitato e' disponibile anche `ui network`. Le schermate del QR
nei tre temi sono state catturate dal TFT e decodificate con ZXing-C++,
verificando che restituiscano lo stesso payload Wi-Fi completo. Acquisizioni
locali: `.pio/wifi-previews/`; log: `.pio/wifi-previews/review.log`.

Le anteprime `ui indicators ready|warning|error|off` mostrano i diversi stati
di colore e la X senza scollegare la SD, cambiare Wi-Fi o alterare lo stato
dei servizi. Per tornare ai dati effettivi usare `ui home`. Le anteprime sono
escluse dalla build normale insieme agli altri comandi diagnostici.
I quattro stati sono stati acquisiti e verificati nei tre temi, incluso il
blocco di Gioca per SD indisponibile/errore. Schermate e log locali:
`.pio/status-previews/`. I test del QR continuano a decodificare il payload
Wi-Fi completo. ELF e BIN della build finale: `.pio/wifi-symbols/final.*`.
Upload e ultimo avvio: `.pio/status-final-upload.log` e
`.pio/status-final-boot.log`, con Wi-Fi e portale attivi e nessun panic.
L'ultimo avvio contiene ancora errori SD CMD17/CMD13: gli indicatori mostrano
gli esiti I/O osservati, senza pretendere di risolvere il guasto di lettura.

Per aggiornare anche nome e logo del portale web eseguire `npm run build`
dalla cartella `frontend` e copiare il contenuto di `sd/.core/portal` nella
cartella `/.core/portal` della microSD. Il solo upload del firmware non
aggiorna quei file. La build web e il controllo TypeScript sono stati
eseguiti; il PC di verifica non era collegato all'access point del dispositivo.

Ripristinare `UI_DIAGNOSTICS_ENABLED=0` e ricaricare al termine delle prove.
Il flag e' disabilitato nella configurazione normale; in release i comandi
sono comunque esclusi. Per la storia delle anomalie SD e i limiti delle
verifiche vedere [SD_DIAGNOSTICS.md](SD_DIAGNOSTICS.md).

Per la prova di regressione monitor CPU + salvataggi NVS vedere
[UI_STABILITY.md](UI_STABILITY.md) e `scripts/ui_stability.py`.
