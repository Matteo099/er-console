# Giochi in italiano e inglese

La lingua scelta in Impostazioni governa catalogo, titolo della partita e testi
degli script dei cinque giochi in `sd/games`. I contenuti dei giochi hanno
dizionari propri; i testi del sistema restano in `Translations.def`.

## Metadati

Ogni `config.json` conserva i campi precedenti e aggiunge:

```json
{
  "name": "Test - Ciclo",
  "description": "Gioco di prova del ciclo di aggiornamento.",
  "translations": {
    "it": {"name": "Test - Ciclo", "description": "Gioco di prova del ciclo di aggiornamento."},
    "en": {"name": "Test - Loop", "description": "Test game for the update loop."}
  }
}
```

`GameMetadata` legge entrambe le lingue. `GameInfo` le conserva nella cache del
catalogo; le View scelgono la variante corrente senza rileggere la SD. Se una
traduzione manca, restano utilizzabili i campi precedenti `name`/`description`.
ID delle cartelle, versioni, durata, difficolta', immagini e configurazione degli
ingressi non dipendono dalla lingua.

## Testi Lua

Ogni script raccoglie i messaggi in un dizionario e li risolve con la stessa
lingua del firmware:

```lua
local messages = {
    map = {it = "Mappa", en = "Map"},
    channel = {it = "Canale %d premuto", en = "Channel %d pressed"},
}
local function text(key)
    return system.translate(messages[key])
end

local title = text("map")
local message = string.format(text("channel"), 1)
```

`system.translate` usa `it` o `en`, quindi prova italiano e inglese come fallback.
La funzione legge la lingua attuale a ogni chiamata. I parametri di
`string.format` devono corrispondere nelle due versioni. `system.language()`
resta disponibile per contenuti specifici (`it-IT` / `en-GB`).

Nel test Oggetti, titoli e paragrafi sostituiscono i precedenti placeholder;
gli oggetti vengono ripubblicati anche se la lingua cambia durante la pausa,
senza azzerare i progressi della partita. Le immagini restano gli stessi asset.
Gli script prototipo 1/2 conservano le rispettive API e la logica hardware:
questa modifica ne localizza i testi, senza convertirli alle API dei test moderni.

## Aggiornare la scheda

L'upload del firmware aggiorna il lettore dei metadati e `system.translate`,
ma **non copia i giochi sulla microSD**. Copiare anche `config.json` e `game.lua`
aggiornati per tutte le cartelle di `sd/games` in `/games` sulla scheda fisica.
Riavviare dopo la copia per caricare il nuovo catalogo. Gli script aggiornati
richiedono il firmware che espone `system.translate`.

## Verifiche

```powershell
python scripts/check_game_translations.py
```

Il controllo verifica i cinque metadati bilingui, tutti i messaggi Lua, chiavi,
placeholder printf e caratteri supportati dai font di lettura.

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`:

```powershell
python scripts/ui_settings.py --reset --cycles 3
python scripts/ui_game_translations.py
```

Il primo test trascina gli slider attraverso il lettore touch LVGL e rilascia
il puntatore con coordinate grezze `(0,0)`. Controlla valori, persistenza e
ripristino delle preferenze iniziali. `ui set` resta disponibile per ripristinare
i controlli, ma non viene usato per la verifica del trascinamento.

Il secondo invia gli asset del PC come fixture in RAM: usa parser, View del
catalogo e interprete Lua del dispositivo in entrambe le lingue. Esegue
init/update del solo esempio Oggetti con ingressi simulati. Non scrive sulla SD,
non avvia le partite hardware e non cambia le credenziali di rete. Ripristinare
il flag diagnostico a zero dopo le prove.

## Verifiche del 7 novembre 2026

- Corretto il percorso di rilascio touch: il driver restituisce `(0,0,false)`
  quando non e' premuto; `TouchInputState` conserva l'ultima coordinata valida.
  LVGL 9.4 usa la posizione anche in `LV_EVENT_RELEASED` per completare lo slider.
- 27 controlli sul dispositivo superati, inclusi touch, metadati bilingui,
  fallback Lua e preferenze. La regressione trascina entrambi gli slider in
  entrambe le direzioni e verifica la persistenza dopo reset. Tre cicli di
  navigazione completati, con preferenze originali ripristinate.
- Cinque config e 47 messaggi Lua verificati staticamente. Tutti i giochi
  controllati in italiano e inglese nel parser e nell'interprete dell'ESP32;
  il test Oggetti verifica anche il cambio lingua durante la pausa con input
  simulati. Nessun gioco hardware avviato e nessuna scrittura SD in queste prove.
- Acquisizioni del catalogo IT/EN: `.pio/game-translations/catalog-*.png`;
  log `.pio/game-translations/session.log`, `.pio/touch-volume-regression/session.log`.

Wi-Fi e portale restano disabilitati come nella configurazione precedente.
L'upload finale ripristina anche `UI_DIAGNOSTICS_ENABLED=0`.
Build e upload finali riusciti; avvio osservato per 22 secondi senza panic,
con SD montata e menu raggiunto a 1650 ms. Log:
`.pio/touch-games-final-upload.log`, `.pio/touch-games-final-boot.log`.
ELF/BIN corrispondenti in `.pio/touch-games-symbols/final.*`.
Archivio per la copia sulla scheda: `.pio/games-it-en.zip` (contiene `/games`).
