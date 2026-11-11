# Preparazione dei contenuti di gioco

La schermata di preparazione anticipa le letture dei contenuti della cartella
del gioco, comprese le sottocartelle. `game.lua` ha la precedenza, poi gli altri
script Lua e i file piu' piccoli. Le immagini, JSON e altri dati sono conservati
come byte originali in una cache condivisa, utilizzata da `StorageSystem` e dal
filesystem LVGL `S:`. Non vengono copiati soltanto per scaldare la SD: le
successive letture di Lua e delle immagini usano effettivamente la RAM.

Un task dedicato esegue scansione e letture; la Scene continua a mostrare la
preparazione. Il lock SPI e' rilasciato fra blocchi di 4 KiB e il task cede la
CPU periodicamente. Timer e sessione statistica iniziano soltanto dopo Avvia
e la conferma del numero di giocatori, come prima.

## Limiti di memoria

| Contenuto | Politica |
| --- | --- |
| Script, immagini e altri file | Massimo 1 MiB complessivo di byte sorgente in PSRAM |
| Singolo file sorgente | Massimo 512 KiB |
| Riserva libera | Almeno 1 MiB di PSRAM e 32 KiB interni prima di una nuova allocazione |
| Scansione | Massimo 128 voci, profondita' di 8 sottocartelle, percorsi delle voci inferiori a 128 byte |
| Attesa della cache sorgente | Massimo 15 secondi; poi i contenuti restanti usano la SD |
| Audio | Budget preesistente separato di 4 MiB; nessuna duplicazione dei WAV nella cache sorgente |

Il budget include i buffer trattenuti da lettori ancora aperti del gioco
precedente. Se manca PSRAM, un file supera il limite o non c'e' spazio, il file
resta leggibile normalmente dalla SD. Una lettura incompleta non viene mai
pubblicata come asset pronto. Gli errori del file principale vengono ancora
gestiti da Lua, con la normale schermata di errore; un contenuto opzionale non
impedisce di avviare il gioco.

Le immagini sono conservate **compresse**, non tutte decodificate in anticipo:
la decodifica PNG e i suoi buffer restano gestiti da LVGL al momento dell'uso.
Il tetto della cache sorgente non e' quindi un tetto di tutta la memoria della
partita: si aggiungono VM Lua, oggetti UI, immagini decodificate e mixer.

## Audio e script

Dopo la preparazione dei file vengono prenotati i tre effetti di sistema
(countdown, sconfitta e vittoria) e fino a quattro WAV scoperti nella cartella
del gioco, privilegiando i piu' piccoli. Rimane uno slot per suoni UI o dinamici.
I limiti di formato, otto sorgenti/cache e budget PCM restano quelli del mixer.
I WAV ulteriori o non precaricabili vengono richiesti normalmente alla loro
riproduzione; un file che supera anche i limiti del mixer non e' riproducibile.
La prenotazione dura per la Scene, incluse le schermate finali, e termina in
uscita o alla nuova preparazione di Riprova.

Lo script principale viene compilato ed inizializzato prima della schermata
pronta. Gli script ausiliari vengono precaricati senza eseguirli: `require`,
`loadfile` e `dofile` passano da `StorageSystem`, quindi possono usare gli stessi
buffer. I percorsi relativi sono risolti nella cartella del gioco; `require`
prova `modulo.lua` e `modulo/init.lua`, anche con nomi puntati come
`helpers.util`. Restano disponibili i normali altri searcher di Lua.
I loader di gioco accettano file nominati, non lo standard input.
Gli accessi nativi tramite `io` e i percorsi esterni calcolati dallo script
restano letture normali: la scansione automatica copre la cartella del gioco.

## Ciclo di vita

`GameAssetCache` mantiene una generazione per preparazione. Uscire, Riprova o
cambiare gioco annullano la pubblicazione dei caricamenti precedenti. I lettori
aperti mantengono un riferimento al proprio buffer, che viene liberato alla
chiusura dell'ultimo lettore. Non si eliminano dati mentre un decoder li usa.

Scritture, rimozioni o spostamenti tramite `StorageSystem` invalidano la cache
sorgente del gioco interessato e fermano la pubblicazione di letture in corso.
Le letture successive tornano sulla SD; eventuali lettori gia' aperti terminano
sul proprio snapshot. Leggere una copia in RAM non azzera gli errori di salute
della SD. I cambiamenti esterni ai servizi del firmware richiedono una nuova
preparazione per aggiornare lo snapshot.

## Verifica

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`:

```powershell
python scripts/ui_game_assets.py
```

`ui assets-test` controlla la cache con gli asset reali di Test - Oggetti senza
eseguirne Lua o inizializzare gli input. Verifica file annidati, sorgente Lua,
firma PNG, seek/EOF, decodifica LVGL, invalidazione, lettori trattenuti,
rilascio della memoria e sostituzione di una preparazione in corso. I loader
Lua ausiliari sono verificati con file di prova in RAM. Non scrive file sulla SD.

Segue il Test - Ciclo reale con input disattivati: attende i contenuti e i suoni,
verifica l'uso della sorgente Lua in cache, il countdown e la sconfitta naturale
con audio gia' pronto. La prova usa statistiche isolate in RAM e master all'1%,
ripristina il master e confronta preferenze e statistiche reali prima e dopo.
`ui assets-state` mostra byte, file, cache hit, esclusioni ed errori.

Verifiche su ESP32 del 7 novembre 2026: **18 controlli superati**. Test -
Oggetti ha preparato quattro file (script, configurazione e due immagini),
96125 byte in 510 ms, senza errori; la decodifica PNG dalla cache e' riuscita.
Nel Test - Ciclo reale il caricamento di Lua ha usato la cache; countdown e
sconfitta naturale hanno avviato l'audio in 0 ms, entrambi cache hit.
Preferenze e statistiche reali sono rimaste identiche. Verificate 163 chiavi
italiano/inglese, firme printf e copertura dei font.
Log: `.pio/game-assets-check/cache/` e `.pio/game-assets-check/game/`.

Build finale compilata e caricata su COM3 con diagnostica UI disattivata,
Wi-Fi `1` e portale `0`. Avvio acquisito per 22 secondi: menu a 1774 ms,
senza errori SD, panic, watchdog o riavvii. Upload e avvio:
`.pio/game-assets-final-upload.log`, `.pio/game-assets-final-boot.log`.
ELF e binario corrispondenti: `.pio/game-assets-symbols/final.*`.
