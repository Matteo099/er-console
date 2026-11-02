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
La dipendenza dal descrittore privato del tema LVGL 9.4 e' isolata in
`UITheme.cpp`: verificarla prima di aggiornare la versione di LVGL.

## Schermate e disponibilita'

- Home: stato microSD, accesso al catalogo, impostazioni e informazioni.
  Il pulsante Gioca e' disabilitato quando il filesystem non e' disponibile.
- Impostazioni: Aspetto per i temi; Sistema per suoni, risorse e rete.
  Il collegamento Wi-Fi esiste solo con `WIFI_ENABLED=1`.
- Rete: stato connessione; accesso al portale e QR solo con entrambi i flag
  `WIFI_ENABLED` e `PORTAL_ENABLED` attivi e con il portale effettivamente avviato.
  Un servizio disabilitato non produce URL o QR dimostrativi.
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

Il cambio tema e suoni viene applicato subito e salvato dopo 400 ms di quiete
o all'uscita dalle impostazioni. Usa il namespace NVS `console-ui`, chiavi
`theme` e `sound`: non riscrive il JSON della SD per ogni tocco. Questo consente
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

Ripristinare `UI_DIAGNOSTICS_ENABLED=0` e ricaricare al termine delle prove.
Il flag e' disabilitato nella configurazione normale; in release i comandi
sono comunque esclusi. Per la storia delle anomalie SD e i limiti delle
verifiche vedere [SD_DIAGNOSTICS.md](SD_DIAGNOSTICS.md).
