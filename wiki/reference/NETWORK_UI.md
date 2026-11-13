# Rete Wi-Fi e password

In Impostazioni > Sistema > Rete, toccare la rete selezionata per aprire **Reti
Wi-Fi**. L'SSID si sceglie dall'elenco, senza tastiera manuale.

Le reti sono ordinate per intensita' decrescente. Ogni riga mostra SSID, quattro
barre di segnale, RSSI in dBm e lucchetto aperto/chiuso con testo **Aperta** /
**Protetta**. Aggiorna richiede una nuova scansione; Indietro chiude l'elenco.
Gli SSID nascosti non compaiono e non vengono aggiunti manualmente.

Scegliendo una rete protetta diversa, la vecchia password viene rimossa dalla
bozza e si apre l'editor. Per una rete aperta non viene richiesta una password.
Le credenziali precedenti rimangono salvate finche' non si conferma la password
o si preme Salva e connetti.

- L'occhio mostra/nasconde il testo. Ogni apertura ripristina la maschera.
- Annulla scarta l'inserimento e torna alla pagina Rete.
- Conferma salva le credenziali e richiede subito la connessione, anche quando
  si usa il tasto di conferma della tastiera. Salva e connetti resta disponibile
  per riprovare o per una rete aperta.
- Conferma e annulla della tastiera restano utilizzabili.

Dopo l'invio, la pagina mostra la scheda di stato con Connessione in corso,
Connesso e indirizzo IP, oppure l'errore di connessione/timeout. Password vuote
per reti protette, credenziali non valide e problemi di salvataggio mostrano
un messaggio senza avviare la connessione. Annulla non salva e non connette.
Con `WIFI_ENABLED=0`, Conferma salva soltanto e il Wi-Fi resta disabilitato.

Intestazione e indicatori SD/Wi-Fi restano visibili. Su TFT 320x240 l'editor
usa campo e occhio da 44 px, azioni da 44 px e tastiera da 108 px. Testi italiani
e inglesi e palette seguono le preferenze dell'interfaccia.

## Configurazione e architettura

All'accensione o al riavvio, con `WIFI_ENABLED=1`, `NetworkSystem` carica le
credenziali salvate e richiede automaticamente la connessione se l'SSID non e'
vuoto. Sono supportate anche reti aperte, senza password. Senza credenziali
non viene tentata alcuna connessione. L'associazione avviene nel task rete:
l'avvio e il funzionamento della console proseguono anche se la rete non e'
disponibile. Dopo 10 secondi senza connessione viene segnalato il timeout;
si puo' riprovare dalla pagina Rete.

La scansione richiede `WIFI_ENABLED=1` in `src/config/Console.h`; il portale puo'
restare spento. Con Wi-Fi disabilitato la lista mostra lo stato esplicito,
Aggiorna e' disabilitato e nessuna scansione SDK parte.

`NetworkSystem` esegue scansioni asincrone e conserva i risultati. Il task rete
e' l'unico proprietario del buffer Arduino: anche il portale usa questa cache,
senza chiamate concorrenti a `scanNetworks` / `scanDelete`. Il contratto HTTP
precedente rimane sincrono sul task del portale, mentre il task UI resta libero.
Una connessione richiesta durante la scansione ne attende il completamento;
il timeout di associazione parte quando viene eseguito `WiFi.begin`.

Il modello elimina SSID vuoti, conserva l'AP piu' forte per SSID e protezione e
limita l'elenco alle 24 reti piu' forti. La UI riceve una copia solo al cambio
della revisione. Le soglie delle quattro barre sono -55/-67/-80 dBm.

`NetworkScene` gestisce il servizio, `NetworkView` la bozza, `WifiNetworkPicker`
l'elenco e `TextInputDialog` l'inserimento. Le icone sono forme LVGL, senza nuove
immagini SD o dipendenze da glifi.

## Verifica

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`:

```powershell
python scripts/diagnostics/ui_network.py
```

Per verificare anche l'associazione reale e i due flussi automatici:

```powershell
python scripts/diagnostics/ui_network_connection.py --port COM3
```

Questa seconda prova usa il servizio rete e la pagina effettivi. Sostituisce
temporaneamente le credenziali salvate con una configurazione vuota e una rete
inesistente, riavvia il dispositivo e verifica che avvio e UI proseguano. Un
backup esatto della configurazione originale resta nel namespace NVS `wifi-check-v2`; il
blocco `finally` lo ripristina, senza trasferire password al PC. Non modificare
la configurazione di rete manualmente durante la prova. Dopo il ripristino,
se la rete originale e' raggiungibile, verifica connessione al boot, Conferma
della password, messaggio di successo/IP e stabilita' della connessione.

In caso di interruzione del processo, con la stessa build diagnostica inviare
`ui wifi-fixture restore` tramite seriale, quindi riavviare la console. I comandi
fixture e stato sono esclusi dalle build normali. `ui wifi-log off/on` sospende
e ripristina soltanto l'output di debug ROM/Arduino durante le acquisizioni:
quei log bypassano il mutex di `HardwareSerial` e possono spezzare una risposta
seriale. Non modifica il comportamento del servizio Wi-Fi. Al termine delle
prove ripristinare `UI_DIAGNOSTICS_ENABLED=0` e caricare il firmware normale.

La prova usa reti/password dimostrative e tocca i controlli attraverso il
lettore LVGL. Verifica selezione protetta/aperta, occhio, Annulla/Conferma,
stati scansione e navigazione in entrambe le lingue. Le Scene di anteprima non
salvano credenziali e non attivano Wi-Fi. Il log degli oggetti oscura il contenuto
dei campi anche con l'occhio attivo. Ripristinare il flag diagnostico a zero
dopo le prove. Questi test con radio spenta non verificano ricezione RF,
associazione al router o portale via HTTP.

## Verifiche del 7 novembre 2026

- 31 controlli sul dispositivo superati, inclusi ordinamento/deduplicazione,
  soglie RSSI, limite risultati e rispetto del flag Wi-Fi disabilitato.
- UI provata in IT/EN con eventi reali del puntatore: selezione protetta/aperta,
  maschera e occhio, Annulla senza applicare, Conferma della bozza e stati vuoto,
  ricerca, occupato, errore e disabilitato. Credenziali salvate non modificate.
- Varianti `(Wi-Fi, portale)` `(0,1)`, `(1,0)`, `(1,1)` compilate sintatticamente,
  inclusi servizio rete, Scene/View e servizio portale condiviso.
- Indicatori SD/Wi-Fi sul layer superiore, persistenti tra scene e sopra i
  dialoghi. Le acquisizioni seriali compongono schermata e layer superiore,
  come il TFT.

Log e schermate locali: `.pio/network-ui-final/`, `.pio/network-header/`;
matrice `.pio/network-picker-matrix-final.log`. La verifica RF rimane esclusa
perche' Wi-Fi e portale sono rimasti a zero. La build finale ripristina la
diagnostica seriale a zero.
Build e upload finali riusciti; avvio osservato per 22 secondi senza panic,
con SD montata e menu a 1655 ms. Log: `.pio/network-picker-final-upload.log` e
`.pio/network-picker-final-boot.log`; ELF/BIN in `.pio/network-picker-symbols/`.

## Verifiche dell'11 novembre 2026

Verificati sul dispositivo i commit `7f5a4dd` e `ccf6610`, con Wi-Fi abilitato
e portale disabilitato:

- Senza credenziali: nessuna associazione richiesta, nessun falso errore;
  avvio completato in 1786 ms.
- Credenziali per una rete assente: avvio completato in 1779 ms, UI utilizzabile
  durante il tentativo e timeout visualizzato dopo circa 10 secondi.
- Annulla non avvia una nuova associazione. Conferma della password avvia
  subito il tentativo senza un secondo clic su Salva e connetti; la pagina
  mostra progresso e poi errore se la rete non risponde.
- Con la rete reale salvata: avvio e connessione automatica con IP riusciti
  (boot completato in 2115 ms). Riconferma della password riuscita e connessione
  verificata stabile, con messaggio Connessa e indirizzo IP.
- Anche il tasto di conferma della tastiera LVGL avvia la connessione: prova
  con tocchi reali riuscita, senza premere Salva e connetti.
- Credenziali originali ripristinate e preferenze UI/audio preservate.
- Regressione del selettore e dell'editor superata sia in italiano sia in
  inglese, inclusi occhio, annullamento e stati della scansione.

Log e schermate: `.pio/wifi-workflows-verified/` e
`.pio/wifi-workflows-verified-result.log`; prova della tastiera in
`.pio/wifi-keyboard-check/`, regressione IT/EN in `.pio/wifi-ui-regression/`.
Il codice funzionale delle due feature
non ha richiesto correzioni; le aggiunte riguardano la diagnostica ripetibile.
Build e upload finali riusciti con `UI_DIAGNOSTICS_ENABLED=0`, `WIFI_ENABLED=1`
e `PORTAL_ENABLED=0`. Nell'ultimo avvio la rete ha assegnato un IP a 2068 ms e
il boot si e' completato a 2082 ms; acquisizione di 18 secondi senza panic,
brownout, watchdog o errori SD: `.pio/wifi-features-final-boot.log`.
