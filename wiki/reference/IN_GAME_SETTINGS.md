# Impostazioni audio durante la partita

L'ingranaggio nella barra inferiore, accanto agli altri comandi della partita,
apre un pannello con volume e
ON/OFF indipendenti per Musica ed Effetti. E' disponibile nella schermata
principale e nella vista degli oggetti, quando il gioco e' pronto, in corso
o in pausa. Durante caricamento, conferma di abbandono e schermate finali
il comando non e' disponibile.

Il pannello e' una `InGameSettingsSubview` posseduta dalla stessa `GameView`.
Non richiede un cambio di Scene, non ricarica lo script e non chiama pause o
resume su Lua. Aprendolo mentre si gioca, il timer e lo script continuano;
aprendolo durante una pausa, la partita resta in pausa anche dopo la chiusura.
"Torna al gioco" nasconde soltanto il pannello, mantenendo la vista principale
o degli oggetti precedentemente selezionata, con i suoi widget e posizioni.

Il timer rimane nel footer fisso, insieme al pulsante di ritorno, anche quando
si scorrono i controlli. Gli indicatori globali SD e Wi-Fi restano visibili.
La superficie del pannello intercetta i tocchi: i comandi sottostanti di avvio,
pausa e abbandono non ricevono input attraverso le impostazioni.

## Applicazione e persistenza

I controlli riusano `AudioPreferenceSubview` e pubblicano gli stessi eventi
delle Impostazioni standard. `GameScene` applica subito le preferenze al mixer
tramite mailbox e aggiorna il feedback UI. Il breve riscontro del cursore usa
`VolumeFeedback`, con il limite esistente di un tono ogni 250 ms.
Aprire o chiudere il pannello non ferma le altre sorgenti audio; modificare
ON/OFF o volume mantiene il normale comportamento del gruppo selezionato.

Le modifiche vengono salvate in NVS dopo 400 ms senza ulteriori cambiamenti o
alla chiusura. Si leggono le preferenze piu' recenti e si aggiornano soltanto
i due gruppi audio, preservando tema, lingua e configurazione di rete.
I dati di gioco e i file sulla SD non vengono riscritti.

Una vittoria, la scadenza del tempo, un errore di caricamento o l'uscita dalla
Scene chiudono il pannello, salvano le modifiche pendenti e fermano soltanto
l'eventuale tono di anteprima prima di proseguire il normale evento di gioco.
Il pannello non impedisce quindi il completamento o la scadenza della partita.

## Verifica su ESP32

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`:

```powershell
python scripts/diagnostics/ui_ingame_settings.py
```

La suite `ui ingame-test` esegue 13 controlli LVGL su un display isolato:
pulsante visibile e dimensionato nella barra inferiore del timer e degli
oggetti, assenza di sovrapposizioni, emissione degli eventi, identita' della
schermata, ritorno alla vista principale o degli oggetti, intercettazione
dell'input e chiusura del pannello sugli stati non giocabili.

Lo script esegue poi il vero Test - Ciclo da un minuto, con tutti gli input
hardware disattivati. Verifica i trascinamenti sui cursori, applicazione e
persistenza dei due volumi, prosecuzione di timer e Lua, conservazione della
pausa e scadenza naturale mentre il pannello e' aperto. Controlla che Scene e
script siano stati caricati una sola volta. Durante le prove il master e'
temporaneamente all'1%; le preferenze iniziali vengono salvate in un file
locale e ripristinate prima della scadenza e anche nel percorso di errore.

Verifiche del 7 novembre 2026: tutti gli 11 controlli e la partita reale sono
passati; le schermate a 320x240 sono state ispezionate. Il catalogo contiene
143 chiavi valide in italiano e inglese, con copertura dei font.
Log e immagini: `.pio/ingame-settings-check/`.

Build finale caricata su COM3 con diagnostica disattivata; Wi-Fi `1` e portale
`0` conservati. Avvio osservato per 22 secondi, menu a 1765 ms, senza errori SD,
panic o riavvii durante l'acquisizione. Upload e boot:
`.pio/ingame-final-upload.log`, `.pio/ingame-final-boot.log`.
ELF/BIN corrispondenti: `.pio/ingame-symbols/final.*`.

Aggiornamento barra inferiore: 13 controlli LVGL superati, inclusa la
disposizione completa con il pulsante oggetti, il footer degli oggetti e il
ritorno al timer. La prova del Test - Ciclo ha confermato volumi immediati,
conservazione della pausa e chiusura automatica alla scadenza. Preferenze e
statistiche reali sono state preservate. Log e schermate 320x240, inclusa
`game-controls.png`: `.pio/ingame-footer-check/`.
