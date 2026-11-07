# Impostazioni centralizzate

La schermata TFT mantiene la struttura Scene/View. `SettingsScene` gestisce
preferenze e servizi; `SettingsView` dispone quattro schede; i due
`AudioPreferenceSubview` condividono lo stesso componente per volume e interruttore.

- **Tema**: Menta, Cielo e Corallo, applicati senza ricreare le scene.
- **Audio**: volume 0–100 e ON/OFF indipendenti per musica ed effetti.
- **Lingua**: italiano predefinito, inglese selezionabile e applicato subito.
- **Sistema**: risorse CPU/RAM/PSRAM/SD e configurazione della rete.

Indietro da Risorse o Rete torna alla scheda Sistema. Il contenuto delle schede
scorre, mentre intestazione, selettore delle schede e Indietro restano visibili.

## Persistenza e compatibilita'

`DeviceConfigRepository` combina default, JSON SD e preferenze NVS. Le modifiche
TFT vengono salvate dopo 400 ms di inattivita' o uscendo dalla schermata, senza
scrivere sulla SD. Un singolo record versionato `console-ui/prefs-v2` conserva
tema, lingua e i quattro valori audio. Le vecchie chiavi `theme` e `sound` e i
campi JSON `audio.volume` / `audio.enabled` sono ancora leggibili. Il vecchio
interruttore OFF migra entrambi i canali su OFF. Il portale legge e scrive lo
stesso modello, includendo i vecchi campi come alias degli effetti.

```json
{
  "locale": {"language": "it-IT"},
  "ui": {"theme": "mint"},
  "audio": {
    "music": {"enabled": true, "volume": 50},
    "effects": {"enabled": true, "volume": 70}
  }
}
```

Il portale salva anche il documento completo sulla SD; le modifiche vengono
propagate al task UI tramite la revisione del repository. Dopo `npm run build`
in `frontend`, copiare `sd/.core/portal` nella cartella `/.core/portal` della
microSD fisica: l'upload del firmware non aggiorna questi file.

## Localizzazione

`src/systems/ui/localization/Translations.def` e' il catalogo unico del firmware:
ogni riga definisce chiave stabile, italiano e inglese. Usare `tr(Text::Settings)`
per testi statici e `I18n::format(Text::GameDetails, minuti, livello)` per valori.
Non aggiungere stringhe visibili direttamente alle View.

`Label`, `Button` e `BrandHeader` registrano automaticamente i testi del catalogo;
`LocalizedText` aggiorna gli oggetti esistenti e rimuove i riferimenti quando LVGL
li elimina. Per testi concatenati usare un binding esplicito oppure aggiornare
il testo in `Scene::onPreferencesChanged()`. Tutte le operazioni LVGL restano nel
task UI; selezionare la lingua non ricrea la partita o ne azzera il timer.

Le storie, i titoli e i messaggi scritti dagli autori rimangono dati del gioco:
i cinque esempi includono ora entrambe le lingue nei metadati e nei dizionari Lua.
Gli script usano `system.translate({it=..., en=...})` o `system.language()`.
Schema, compatibilita' e copia sulla scheda: [GAME_LOCALIZATION.md](GAME_LOCALIZATION.md).
Log tecnici e nomi dei protocolli restano stabili.
Il portale usa i cataloghi Vue i18n esistenti, con italiano predefinito; la
selezione della lingua del dispositivo offre italiano e inglese.

Verificare il catalogo con `python scripts/check_translations.py`: controlla
chiavi duplicate, traduzioni mancanti e firme printf.
Controlla anche che i caratteri siano compresi nei font di lettura incorporati.

## Motore audio

`AudioSystem` e' l'unico proprietario di driver, cache e mixer. Le API Lua e le
Scene inviano eventi senza bloccare il task UI; le preferenze usano una mailbox
che mantiene l'ultimo valore anche con la coda degli effetti piena. Stop usa
ID con generazione per scartare riproduzioni precedenti ancora in coda.

`AudioMixer` miscela fino a otto voci PCM, applica volumi per sorgente e gruppo, converte mono/
stereo e frequenza e limita il risultato per evitare overflow. Supporta WAV PCM
16 bit, 8–96 kHz, con uscita mono a 22050 Hz. Non esegue allocazioni nel render.
La cache non elimina i file ancora usati dalle voci. Le letture SD a freddo
restano sincrone nel task audio: una scheda lenta o in errore puo' ancora causare
interruzioni, indipendentemente dai controlli volume.

```lua
audio.play("/games/demo/music.wav", true) -- musica
audio.play("/games/demo/hint.wav")        -- effetto
audio.play("/games/demo/alarm.wav", true, "effects")
audio.stop("music")                       -- lascia attivi gli effetti
audio.stop()                              -- entrambi
```

Il suono dei pulsanti, i beep e l'allarme del timer sono effetti. Disattivare gli
effetti interrompe i suoni singoli e sospende i loop; disattivare la musica ne conserva la
posizione, anche se il brano viene selezionato mentre il canale e' disattivato.
`audio.setVolume()` rimane il controllo master compatibile con gli
script esistenti, separato dalle preferenze dei canali.

API per sorgente, limiti della cache e preparazione degli asset:
[AUDIO.md](AUDIO.md).

## Rete e diagnostica

L'SSID si seleziona dalla lista delle reti disponibili, con segnale e protezione.
La password usa una tastiera con occhio, Annulla e Conferma; viene inizialmente
mascherata. Salva valida SSID (massimo 32 byte), password aperta/vuota, 8–63 byte
oppure PSK esadecimale di 64 cifre. Le credenziali sono un record NVS separato
`console-net/credentials` e restano utilizzabili senza SD.

Con `WIFI_ENABLED=1`, Salva e connetti e Disconnetti inviano richieste al task
rete. La pagina mostra stato, SSID, IP e segnale RSSI; il timeout e' 10 secondi.
Disconnetti preserva l'access point del portale. QR e URL richiedono anche
`PORTAL_ENABLED=1` e il servizio avviato. I flag restano attualmente a zero per
l'isolamento dei brownout con alimentazione USB del PC.
Per la selezione delle reti, i controlli e la scansione asincrona vedere
[NETWORK_UI.md](NETWORK_UI.md).

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`, `ui selftest` verifica mixer,
migrazione, validazione credenziali e traduzione live senza audio o scritture NVS.
`ui preferences` riporta le preferenze non sensibili; `ui set x y valore` agisce
sui controlli LVGL reali e pubblica il relativo evento. Ripristinare il flag a
zero dopo le prove.

La regressione completa e' `python scripts/ui_settings.py --reset`: usa i
controlli LVGL reali, verifica i due canali, cambia lingua, riavvia per verificare
NVS e prova il ritorno da Risorse/Rete alla scheda Sistema. Salva le preferenze
iniziali nel log e tenta di ripristinarle anche in caso di errore. Il comando
storico `scripts/ui_stability.py` usa la stessa procedura senza riavvio.

## Verifiche del 6 novembre 2026

- Build debug ESP32 e upload COM3 riusciti; build portale e TypeScript riusciti.
- Catalogo: 127 chiavi verificate in entrambe le lingue, con firme printf uguali.
- Sul dispositivo: 19 controlli mixer/migrazione/localizzazione superati.
- Preferenze dei due canali modificate separatamente tramite switch e slider
  reali; persistenza e avvio inglese verificati dopo reset hardware.
- Cinque cicli Risorse/Rete e tre ulteriori cicli con cambio tema e salvataggi
  NVS durante il monitoraggio CPU, senza panic; preferenze iniziali ripristinate.
- Form e tastiera password acquisiti su TFT 320x240; password mascherata,
  indicatori SD/Wi-Fi visibili, scheda Language su una riga.
- Compilazione sintattica delle varianti `(Wi-Fi, portale)` `(0,1)`, `(1,0)`,
  `(1,1)`, incluse gestione rete e API configurazione. La connessione radio e
  le API HTTP non sono state provate sul dispositivo: entrambi i flag restano 0.

Log e schermate locali: `.pio/settings-regression/`, `.pio/settings-stability/`,
`.pio/settings-layout-review/`; build web `.pio/settings-frontend-build.log`.
Queste prove non certificano l'assenza dei precedenti errori SD o brownout.
L'ultima build e' caricata con diagnostica seriale disabilitata; l'acquisizione
finale di 22 secondi raggiunge il menu a 1649 ms, con SD montata e senza panic.
In un'acquisizione precedente di questa sessione sono ricomparsi CMD17/CMD13.
Upload e boot: `.pio/settings-final-upload.log`, `.pio/settings-final-boot.log`.
ELF/BIN corrispondenti: `.pio/settings-symbols/final.*`.
