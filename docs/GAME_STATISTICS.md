# Statistiche e giocatori

Le statistiche globali sono in **Impostazioni > Sistema > Statistiche**.
Il pulsante con l'icona elenco nell'intestazione del catalogo apre le statistiche
del gioco selezionato. Indietro torna alla scheda Sistema o allo stesso gioco
nel catalogo, secondo il punto di ingresso.

## Selezione dei giocatori

Premendo Avvia nella schermata pronta della partita compare un selettore con
frecce su e giu', Annulla e Avvia. Il valore minimo e' sempre 1; il tipo salvato
e' un intero a 16 bit, con limite superiore 65535 e freccia disabilitata al limite.
Solo confermando il selettore viene creata la sessione e avviato il timer/Lua.
Preparazione, caricamento e attesa nel selettore non consumano tempo di gioco.
Annulla non crea statistiche. Riprendere dalla pausa non chiede nuovamente il
numero; una nuova partita o Riprova richiedono invece una nuova conferma.
Il gruppo precedente viene proposto come default dopo Riprova nella stessa Scene.

Il numero scelto e' associato alla sessione e mostrato anche nel riepilogo
di vittoria. La durata del riepilogo usa lo stesso conteggio delle statistiche.

## Definizioni dei contatori

| Dato | Significato |
| --- | --- |
| Partite giocate | Sessioni effettivamente avviate dopo la conferma dei giocatori |
| Vittorie | Completamenti richiesti dallo script dopo l'avvio |
| Sconfitte | Scadenze del timer |
| Abbandonate | Uscite confermate, errori dopo l'avvio e sessioni interrotte da reset |
| In corso | Sessione avviata e non ancora conclusa |
| Durata media | Media ponderata del tempo attivo delle vittorie e sconfitte; abbandoni e sessioni in corso esclusi |
| Vittoria piu' rapida / lenta | Minimo e massimo tempo attivo delle sole vittorie |
| Partecipanti totali | Somma dei giocatori selezionati per ogni sessione, anche abbandonata; non persone uniche |
| Ultimo gruppo | Numero di giocatori dell'ultima sessione avviata |

Prima di una vittoria, i record vengono mostrati con `-`; prima di una partita
conclusa, anche la media e' `-`. Una vittoria molto breve puo' essere `00:00:00`:
i millisecondi sono conservati e solo la visualizzazione e' in secondi.
Ogni risultato e' registrato una sola volta, anche se arrivano notifiche Lua
ripetute o la Scene viene successivamente chiusa. Una sessione non avviata non
puo' produrre una vittoria statistica.

`GameSessionClock` accumula separatamente millisecondi attivi e in pausa e
gestisce il rollover di `millis()`. Il tempo trascorso nelle impostazioni
in gioco e' attivo, come il timer; il dialogo di abbandono segue invece la
pausa gia' prevista dal gioco. La sconfitta e' limitata alla durata programmata.
I dati delle partite precedenti a questo firmware, presenti soltanto nella
vecchia struttura RAM della Scene, non possono essere ricostruiti.

## Persistenza

`GameStatistics` e' il modello puro; `StatisticsRepository` gestisce un unico
snapshot versionato nel namespace NVS `game-stats`, chiave `snapshot`.
Non usa la SD e non modifica configurazioni, salvataggi o file Lua dei giochi.
Identita' del gioco e' l'ID della cartella: cambiarne il titolo mantiene le
statistiche, rinominare la cartella crea invece un nuovo ID.

Lo snapshot comprende riepilogo globale, riepiloghi per gioco, sessione attiva
e ultime **32 sessioni** con ID, gioco, numero di giocatori, risultato e durata.
Gli aggregati coprono tutta la vita delle statistiche, anche quando lo storico
recente supera 32 record. Una sessione attiva viene salvata prima dell'avvio;
alla successiva apertura dopo un reset e' chiusa una sola volta come interrotta,
con durata sconosciuta esclusa dalla media. Il risultato viene salvato alla fine.
Non vengono effettuate scritture ogni secondo.

Un solo blob conserva insieme contatori globali, per gioco e sessioni,
evitando aggiornamenti parziali fra chiavi diverse. Il decoder verifica schema,
tipi, conteggi, ID e coerenza degli aggregati prima di accettare uno snapshot.

Il limite conservativo dello snapshot e' **6000 byte**, per lasciare spazio
al ricambio dei blob NVS e alle preferenze esistenti. Il numero di giochi
salvabili dipende dalla lunghezza degli ID e dallo storico. Se NVS e' pieno,
la scrittura fallisce o il limite viene superato, i contatori continuano in RAM
e la pagina segnala "Statistiche temporanee"; le modifiche non salvate vengono
perse al reset. Non e' presente un comando pubblico di cancellazione dei dati.

## Verifica

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`:

```powershell
python scripts/ui_statistics.py
```

`ui stats-test` verifica il modello, i record, la media, i gruppi, i casi limite
del selettore, pause frazionarie, rollover, recupero dopo riapertura, snapshot
invalidi e scritture fallite. Il test di persistenza usa soltanto il namespace
NVS `stats-check` e pulisce la propria chiave `snapshot` alla fine.

Le prove UI usano il vero Test - Ciclo con input hardware disattivati e un
archivio statistico in RAM selezionato solo dalla diagnostica. Verificano una
vittoria con 3 giocatori e una pausa, una sconfitta naturale da un minuto con
1 giocatore, un abbandono con 2 giocatori, selezione annullata, ripresa senza
nuova selezione, riepiloghi e navigazione di ritorno. Il master e' all'1%
durante le prove e le statistiche reali vengono confrontate prima e dopo.

Verifiche del 7 novembre 2026: **26 controlli superati**, compreso il vero NVS
di prova; tutti i flussi UI elencati superati. Le schermate 320x240 di selettore,
vittoria e statistiche sono state ispezionate. Il catalogo contiene 162 chiavi
valide in italiano e inglese, con firme printf e copertura dei font verificate.
Log e schermate: `.pio/statistics-unit/`, `.pio/statistics-check/`.

La build finale debug e' stata compilata e caricata su COM3 con diagnostica UI
disattivata, Wi-Fi attivo e portale disattivato. Nel successivo avvio di 22
secondi non sono comparsi panic, watchdog, brownout o errori di lettura SD.
Log: `.pio/statistics-final-upload.log`, `.pio/statistics-final-boot.log`.
ELF e binario corrispondenti: `.pio/statistics-symbols/final.elf` e `final.bin`.
