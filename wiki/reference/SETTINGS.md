# Impostazioni centralizzate

La schermata TFT mantiene la struttura Scene/View. `SettingsScene` gestisce
preferenze e servizi; `SettingsView` dispone quattro schede; i due
`AudioPreferenceSubview` condividono lo stesso componente per volume e interruttore.

Durante una partita l'ingranaggio apre gli stessi controlli audio sopra la
GameView, conservando stato, timer e script: [IN_GAME_SETTINGS.md](IN_GAME_SETTINGS.md).

- **Tema**: Menta, Cielo e Corallo, applicati senza ricreare le scene.
- **Audio**: volume 0–100 e ON/OFF indipendenti per musica ed effetti.
- **Lingua**: italiano predefinito, inglese selezionabile e applicato subito.
- **Sistema**: risorse CPU/RAM/PSRAM/SD e configurazione della rete.
- **Statistiche**: riepiloghi globali dalla scheda Sistema e per gioco dal
  catalogo; [GAME_STATISTICS.md](GAME_STATISTICS.md).
- **Aggiornamenti**: versione installata, GitHub Releases, download e
  installazione OTA dalla scheda Sistema; [wiki/OTA.md](../OTA.md).

Indietro da Risorse o Rete torna alla scheda Sistema. Il contenuto delle schede
scorre, mentre intestazione, selettore delle schede e Indietro restano visibili.

## Persistenza e compatibilita'

`DeviceConfigRepository` usa il record NVS versionato `console-cfg/settings` per
tutte le preferenze e configurazioni del dispositivo. Le modifiche TFT vengono
salvate dopo 400 ms di inattivita' o uscendo dalla schermata. I vecchi JSON SD,
`console-ui/prefs-v2`, `theme` e `sound` vengono importati preservando i valori
esistenti; il vecchio interruttore OFF migra entrambi i canali su OFF.
Il portale usa lo stesso modello senza scrivere configurazioni personali sulla SD.
Dettagli e limiti: [DEVICE_SETTINGS_STORAGE.md](DEVICE_SETTINGS_STORAGE.md).

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

Il portale salva il documento completo in NVS; le modifiche vengono
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

Verificare il catalogo con `python scripts/checks/check_translations.py`: controlla
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

I cursori del volume producono un breve tono di riscontro (750 Hz, 40 ms),
con volume proporzionale al livello del canale modificato. L'anteprima Musica
funziona anche con Effetti disattivati e viceversa; zero, OFF e master silenzioso
restano silenziosi. Il tono ha un guadagno per sorgente del 50% per contenere
l'intensita', poi attraversa il normale volume del gruppo e il master.

`AudioPreferenceSubview` pubblica l'intento dell'utente; `SettingsScene` applica
prima la preferenza e usa `VolumeFeedback` per accorpare gli aggiornamenti:
al massimo un tono ogni 250 ms, con riscontro finale per l'ultimo valore non
ancora inviato. Non si ripete un livello invariato. Cambiare scheda, uscire dalle
Impostazioni o disattivare l'audio annulla l'anteprima pendente e la sua sorgente,
senza fermare le altre riproduzioni. Caricare le preferenze o cambiare il solo
interruttore non richiede un tono. La generazione PCM non legge la microSD.

Con diagnostica abilitata, `python scripts/diagnostics/ui_volume_feedback.py` verifica
frequenza delle richieste, ultimo valore, mute, cambi canale, coda piena,
rollover del timer, eventi degli slider e proporzione dei campioni PCM.
La suite usa code isolate e non produce suoni ne' modifica le preferenze.

Verifica del 7 novembre 2026: 25 controlli del feedback, 41 del mixer e 30
generali superati sull'ESP32. `scripts/diagnostics/ui_settings.py --cycles 1` ha verificato
anche i trascinamenti sui cursori reali, con master silenziato e ripristino delle
preferenze iniziali. Build finale caricata su COM3 con diagnostica disattivata;
avvio acquisito per 22 secondi, menu a 1763 ms, senza errori SD o riavvii nel log.
Log in `.pio/volume-feedback-check/`, `.pio/volume-feedback-settings/`,
`.pio/volume-feedback-final-upload.log` e `.pio/volume-feedback-final-boot.log`;
ELF/BIN corrispondenti in `.pio/volume-feedback-symbols/final.*`.

API per sorgente, limiti della cache e preparazione degli asset:
[AUDIO.md](AUDIO.md).

## Rete e diagnostica

L'SSID si seleziona dalla lista delle reti disponibili, con segnale e protezione.
La password usa una tastiera con occhio, Annulla e Conferma; viene inizialmente
mascherata. Salva valida SSID (massimo 32 byte), password aperta/vuota, 8–63 byte
oppure PSK esadecimale di 64 cifre. Le credenziali vengono importate dal vecchio
`console-net/credentials` nel record `console-cfg/settings` e restano
utilizzabili senza SD. Le risposte del portale non restituiscono la password.

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

La regressione completa e' `python scripts/diagnostics/ui_settings.py --reset`: usa i
controlli LVGL reali, verifica i due canali, cambia lingua, riavvia per verificare
NVS e prova il ritorno da Risorse/Rete alla scheda Sistema. Salva le preferenze
iniziali nel log e tenta di ripristinarle anche in caso di errore. Il comando
storico `scripts/diagnostics/ui_stability.py` usa la stessa procedura senza riavvio.

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
