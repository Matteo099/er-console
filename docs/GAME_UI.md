# Avvio dei giochi e conferma di uscita

La struttura resta Scene / View / SubView. `GameScene` gestisce stato,
caricamento, timer e servizi; `GameView` compone le sottoviste e il componente
riutilizzabile `ConfirmationDialog`.

## Caricamento e timer

Un errore SD o Lua all'avvio mostra **Impossibile avviare** e un messaggio
specifico. Riprova ricarica lo stesso gioco. **Tempo scaduto** compare solo
quando il conto alla rovescia di una partita avviata raggiunge zero.
Anche il catalogo distingue l'assenza di giochi da una directory illeggibile.

`LuaSystem` legge interamente lo script prima di compilarlo, con al massimo
tre tentativi per errori di lettura. `IFile::length()` permette di verificare
la lunghezza attesa senza confondere una lettura interrotta con EOF.
Un mutex ricorsivo serializza le operazioni sulla VM tra task Lua e UI;
il ciclo degli input usa un lock analogo durante inizializzazione e arresto.

Le API Lua che completano il gioco o aggiornano gli oggetti pubblicano uno
stato pendente, consumato da `GameScene::onUpdate` sul task UI. Non invocano
LVGL dal task Lua e non lasciano callback asincrone con puntatori a scene
gia' distrutte. All'uscita una scena scarica soltanto lo script che le appartiene.

La durata e' inizializzata da `GameInfo::durationMin`; zero indica una partita
senza limite. Il timer usa il tempo monotono e recupera eventuali secondi
trascorsi fra aggiornamenti, escludendo le pause.

## Abbandono

La X apre **Abbandonare il gioco?** con Continua e Abbandona. Una partita
in esecuzione viene messa in pausa durante la conferma. Continua ripristina
lo stato precedente; Abbandona arresta lo script e torna al catalogo.
La barra SD/Wi-Fi resta visibile sopra il popup.

Disattivare i suoni impedisce anche il caricamento del lungo WAV del timer
e arresta la riproduzione corrente. Il suono di avvio passa dalla stessa
coda degli eventi, dopo l'applicazione della preferenza audio.

## Evidenze hardware e limiti

Durante la prova del 4 novembre 2026 Loop e' stato caricato con durata di
60 secondi e avviato correttamente. In una prima sessione con audio attivo
e' stato acquisito un reset `rst:0xf (BROWNOUT_RST)`:
`.pio/game-baseline/brownout-session.log`. L'utente conferma che ESP32,
TFT e amplificatore sono alimentati tutti dalla USB del PC.

Questo reset indica un evento di tensione sotto soglia; la causa specifica
del calo va verificata sull'alimentazione. Il rilevatore resta abilitato.
Gli errori SD CMD17/CMD13 continuano a essere intermittenti, anche senza
audio: la correzione dell'interfaccia non ne dimostra la risoluzione.
Vedere il [manuale Espressif ESP32-S3](https://documentation.espressif.com/esp32-s3_technical_reference_manual_en.pdf)
per i codici di reset e [SD_DIAGNOSTICS.md](SD_DIAGNOSTICS.md) per il bus.

## Verifiche della revisione

La sessione `.pio/game-review-final/session.log` verifica sulla scheda:

- Inputs caricato con 300 secondi, avvio e abbandono confermato.
- Loop caricato con 60 secondi; annullamento della conferma prima dell'avvio.
- Conferma durante la partita: timer fermo, ripresa e scadenza reale a zero.
- Ripristino della preferenza audio iniziale al termine della prova.

Non si sono verificati panic o reset in questa sessione con audio disattivato
durante le partite. Sono presenti **17** errori `Card Failed`, quindi non
equivale a una prova di stabilita' della SD o dell'alimentazione con audio attivo.
Schermate corrispondenti: `.pio/game-review-final/*.png`.

La navigazione Risorse / Rete torna a Sistema; il QR acquisito in
`.pio/game-review/qr.png` e' stato decodificato. La scansione spazio ha restituito
circa 29,4 MB occupati su 29,1 GB. La build del portale e' riuscita; i rami
Wi-Fi/portale disabilitati sono stati compilati con controllo sintattico.

La configurazione usata per quella verifica mantiene Wi-Fi e portale abilitati e riporta
`UI_DIAGNOSTICS_ENABLED=0`. Usare l'ambiente
`esp32-s3-devkitc-1-n16r8v-debug` anche per il monitor seriale, per mantenere
il decoder allineato all'ELF della build caricata.
La revisione successiva per font e fluidita' disabilita Wi-Fi e portale per
isolare i reset brownout: vedere [UI_PERFORMANCE.md](UI_PERFORMANCE.md).

Upload finale riuscito: `.pio/game-final-upload.log`. ELF e BIN corrispondenti
sono conservati in `.pio/game-symbols/final.*` con hash in
`final-sha256.json` (ELF SHA256 `8881aad0e4d01d29...`). Il successivo avvio di
32 secondi ha raggiunto il menu e avviato il portale a `192.168.4.1`, senza
panic o reset, ma con quattro `Card Failed`:
`.pio/game-final-boot.log`. La porta COM3 e' stata rilasciata al termine.
