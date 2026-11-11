# Rete Wi-Fi e password

In Impostazioni > Sistema > Rete, toccare la rete selezionata per aprire **Reti
Wi-Fi**. L'SSID si sceglie dall'elenco, senza tastiera manuale.

Le reti sono ordinate per intensita' decrescente. Ogni riga mostra SSID, quattro
barre di segnale, RSSI in dBm e lucchetto aperto/chiuso con testo **Aperta** /
**Protetta**. Aggiorna richiede una nuova scansione; Indietro chiude l'elenco.
Gli SSID nascosti non compaiono e non vengono aggiunti manualmente.

Scegliendo una rete protetta diversa, la vecchia password viene rimossa dalla
bozza e si apre l'editor. Per una rete aperta non viene richiesta una password.
Le credenziali precedenti rimangono salvate finche' non si preme Salva e connetti.

- L'occhio mostra/nasconde il testo. Ogni apertura ripristina la maschera.
- Annulla scarta l'inserimento e torna alla pagina Rete.
- Conferma aggiorna la bozza della pagina; l'azione finale per salvare e
  collegarsi rimane Salva e connetti.
- Conferma e annulla della tastiera restano utilizzabili.

Intestazione e indicatori SD/Wi-Fi restano visibili. Su TFT 320x240 l'editor
usa campo e occhio da 44 px, azioni da 44 px e tastiera da 108 px. Testi italiani
e inglesi e palette seguono le preferenze dell'interfaccia.

## Configurazione e architettura

La scansione richiede `WIFI_ENABLED=1` in `src/config/Console.h`; il portale puo'
restare spento. Con Wi-Fi disabilitato la lista mostra lo stato esplicito,
Aggiorna e' disabilitato e nessuna scansione SDK parte. Entrambi i flag sono
rimasti a zero per conservare la configurazione precedente.

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
