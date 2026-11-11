# Anno 2039 — Centrale Nucleare Dolomiti

## Premessa

Nel 2039 la Centrale Nucleare Dolomiti (CND) ospita un reattore sperimentale a fusione di tipo tokamak. Il plasma di idrogeno, ad altissima temperatura, è confinato lontano dalle pareti da un campo magnetico generato da magneti superconduttori, raffreddati all'interno di un criostato.

Durante un test un magnete si danneggia e il criostato si surriscalda. Il sistema di emergenza sigilla il settore per limitare la perdita di elio liquido; gli scienziati vengono evacuati, ma lo spegnimento automatico va in corto circuito. La squadra di manutenzione criogenica deve ripristinare il raffreddamento e i magneti, sigillare la camera e instradare il plasma verso il serbatoio di contenimento prima che la pressione termica provochi un'esplosione.

La centrale, le procedure e lo scarico del plasma sono elementi narrativi del gioco.

## Ambienti e percorso

1. **Anticamera:** formula chimica, ascensore e porta della sala di controllo.
2. **Sala di controllo:** valvole criogeniche, console, stampante, armadio tecnico e comandi delle bobine.
3. **Sala del nocciolo:** modello del toroide, bobine, condotti e serbatoio di contenimento.

La porta della sala del nocciolo è aperta o si apre dopo il ripristino dell'alimentazione nell'enigma 5. Gli enigmi seguono questo ordine:

| # | Enigma | Risultato necessario al seguito |
|---|---|---|
| 1 | Ascensore | Tavola periodica, badge e chiave |
| 2 | Porta della sala di controllo | Accesso alla sala |
| 3 | Raffreddamento del criostato | Formula Nb₃Sn e accesso al vano di servizio |
| 4 | Armadio tecnico | Fusibile |
| 5 | Alimentazione | Diagnosi dei settori danneggiati e accesso al nocciolo |
| 6 | Sostituzione dei moduli | Bobine riparate |
| 7 | Riallineamento delle bobine | Campo magnetico stabile |
| 8 | Chiusura della camera | Camera sigillata |
| 9 | Instradamento del plasma | Serbatoio collegato |
| 10 | Scarico del plasma | Conclusione della missione |

La tavola periodica è suddivisa in quattro parti come materiale di scena. La prima deve contenere almeno Nb e Sn con i rispettivi numeri atomici; la seconda e la terza sono recuperate negli enigmi 3 e 4. La posizione della quarta e l'eventuale uso della tavola completa restano da definire: non sono necessari per completare il percorso descritto. Anche le due chiavi trovate negli enigmi 1 e 3 sono oggetti di scena il cui utilizzo resta da assegnare.

## 1. Apertura dell'ascensore

### Narrazione e obiettivo

La squadra deve aprire l'ascensore di servizio per recuperare l'attrezzatura necessaria ad accedere alla sala di controllo.

### Scenario e indizi

Sulla parete compare la formula **(NH₄)₂[PtCl₆]**. Davanti all'ascensore ci sono quattro pulsanti: una linea, due linee, un quadrato e un esagono. Una targhetta chiarisce la convenzione del gioco: «Leggere da sinistra a destra ogni simbolo chimico e ogni pedice scritto. Per un elemento senza pedice usare 1; leggere separatamente anche il pedice esterno alla parentesi. Scegliere il pulsante con il numero corrispondente di linee o lati».

### Ragionamento e soluzione

Si leggono N = 1, H₄ = 4, il pedice esterno = 2, Pt = 1 e Cl₆ = 6. La sequenza è **una linea → quadrato → due linee → una linea → esagono**. È una convenzione di lettura della formula, non il conteggio totale degli atomi: il pedice esterno non viene moltiplicato per quelli interni.

### Meccanismo fisico e riscontro

I pulsanti registrano la sequenza. Ogni pressione corretta accende un LED verde; un errore accende un LED rosso e azzera la sequenza. Completandola, la porta dell'ascensore si apre.

### Risultato e collegamento

All'interno si trovano il primo quarto della tavola periodica, un badge RFID e una chiave. La tavola serve subito per la porta della sala di controllo; il badge servirà alla console nell'enigma 3.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Pulsante momentaneo sotto cappuccio stampato | 4 | Tactile 6 × 6 mm, normalmente aperto | Una linea, due linee, quadrato, esagono |
| LED verde + LED rosso | 2 | LED 3 mm, 1 per colore | Riscontro della sequenza |
| Resistenza per LED | 2 | 1 kΩ, ¼ W | Una per LED, pilotaggio a 3,3 V |
| Microservo per sblocco porta | 1 | SG90, 5 V | Aziona un chiavistello stampato; apertura manuale |
| Badge RFID | 1 | MIFARE Classic 1K, 13,56 MHz | Consegnato nell'ascensore; letto nell'enigma 3 |

## 2. Porta della sala di controllo

### Narrazione e obiettivo

Lo scanner della porta è guasto: occorre usare l'accesso numerico di emergenza.

### Scenario e indizi

La porta presenta una tastiera, uno scanner per badge e una griglia vuota 2 × 2. Vicino alla formula dell'anticamera si trova un foglio tecnico semibruciato: «Magneti toroidali costruiti con lega di Niobio-Stagno». Una nota sulla griglia indica: «Numeri atomici, nell'ordine indicato sul foglio».

### Ragionamento e soluzione

Sul frammento di tavola periodica si leggono **Nb = 41** e **Sn = 50**. Affiancando i due numeri nell'ordine Niobio-Stagno si ottiene **4150**.

### Meccanismo fisico e riscontro

La tastiera verifica le quattro cifre e apre la porta quando il codice è corretto. Lo scanner guasto non apre la porta e può emettere un suono di accesso negato.

### Risultato e collegamento

La squadra entra nella sala di controllo, dove il monitor segnala **CRIOSTATO OFFLINE** e i circuiti criogenici sono in standby.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Tastiera numerica a matrice | 1 | Membrana 3 × 4, 12 tasti | Ingresso codice, 7 linee digitali |
| Microservo per sblocco porta | 1 | SG90, 5 V | Chiavistello stampato; apertura manuale |
| Scanner guasto di scena | 1 | Pezzo stampato 3D | Non richiede lettore elettronico |

## 3. Ripristino del raffreddamento

### Narrazione e obiettivo

Per fermare il surriscaldamento bisogna riattivare i quattro circuiti criogenici nella sequenza prescritta.

### Scenario e indizi

Ci sono quattro valvole etichettate Elio (He), Azoto (N), Argon (Ar) e Ossigeno (O). Una scheda di emergenza chiede di aprire i circuiti in ordine di temperatura di ebollizione crescente. Una tabella semibruciata riporta He = −269 °C, N = −196 °C e Ar = −186 °C; la riga restante è illeggibile. Una pagina del manuale precisa: «Il circuito dell'Ossigeno deve essere attivato soltanto dopo aver stabilizzato quello dell'Argon».

### Ragionamento e soluzione

I tre valori leggibili si ordinano **He → N → Ar**. L'unico gas mancante è l'ossigeno, che la regola colloca dopo l'argon. La sequenza completa è **He → N → Ar → O**.

### Meccanismo fisico e riscontro

La rotazione di ogni valvola viene rilevata dal sistema. Una valvola aperta fuori sequenza provoca un errore e richiede di riportare tutte le valvole alla posizione iniziale. Alla sequenza corretta i LED di allarme smettono di lampeggiare, si sente il riavvio delle pompe e il monitor mostra **CRIOSTATO STABILIZZATO**, **TEMPERATURA MAGNETI: 4 K** e la richiesta del badge operatore.

Il badge dell'ascensore viene letto dalla console funzionante, distinta dallo scanner guasto della porta. L'autenticazione sblocca l'archivio e un vano di servizio; una stampante può riprodurre il documento visualizzato.

### Risultato e collegamento

Il documento identifica la lega superconduttrice come **Nb₃Sn**. Nel vano si trovano il secondo quarto della tavola periodica e una seconda chiave. La console segnala inoltre **ALIMENTAZIONE MAGNETI INTERROTTA**. La formula serve ad aprire l'armadio tecnico.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Microinterruttore a leva | 4 | KW11-3Z, normalmente aperto | Una camma su ogni valvola rileva la posizione aperta |
| Lettore RFID | 1 | RC522 / MFRC522, 13,56 MHz, SPI, 3,3 V | Legge il badge dell'enigma 1 |
| LED rosso | 2 | LED 3 mm | Allarmi lampeggianti |
| Resistenza per LED | 2 | 1 kΩ, ¼ W | Una per LED |
| Microservo per vano | 1 | SG90, 5 V | Sblocca il vano di servizio |

## 4. Armadio tecnico dei magneti

### Narrazione e obiettivo

Per riparare l'alimentazione serve il fusibile custodito nell'armadio tecnico.

### Scenario e indizi

L'armadio ha una pulsantiera con simboli chimici, tra cui **Nb**, **Sn**, **Fe**, **Cu** e **Ti**. Una lavagna riporta gli stessi simboli su tessere magnetiche. Il documento ottenuto dalla console indica **Nb₃Sn**; una targhetta sull'armadio specifica: «Inserire un simbolo per ogni atomo, nell'ordine della formula».

### Ragionamento e soluzione

La formula contiene tre atomi di niobio e uno di stagno. La sequenza è **Nb → Nb → Nb → Sn**. Ogni simbolo chimico corrisponde a un pulsante, non a due pressioni sulle singole lettere.

### Meccanismo fisico e riscontro

La pulsantiera controlla la sequenza; un errore la azzera. Inserendo i quattro simboli corretti, la serratura si apre.

### Risultato e collegamento

Nell'armadio si trovano un fusibile di sicurezza e il terzo quarto della tavola periodica. Il fusibile serve alla console di alimentazione.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Pulsante momentaneo | 5 | Tactile 6 × 6 mm, normalmente aperto | Nb, Sn, Fe, Cu, Ti; cappucci stampati |
| Microservo per armadio | 1 | SG90, 5 V | Sblocca il chiavistello dell'armadio |

## 5. Ripristino dell'alimentazione

### Narrazione e obiettivo

Il raffreddamento è stabile, ma i magneti non ricevono alimentazione. Occorre sostituire il fusibile e avviare una diagnosi.

### Scenario e indizi

Un portafusibile vuoto, riconoscibile e collegato alla console, è vicino al cavo diretto verso il nocciolo. Sotto la tastiera un promemoria recita: «Numero atomico del Niobio seguito dal numero di atomi di Niobio presenti nel composto».

### Ragionamento e soluzione

La tavola fornisce **Nb = 41** e la formula Nb₃Sn indica **3** atomi di niobio. Dopo aver inserito il fusibile, si digita **413**.

### Meccanismo fisico e riscontro

Il portafusibile rileva l'inserimento; la console abilita la tastiera e verifica il codice. Parte un suono di avviamento, interrotto quasi subito dalla diagnosi. Il monitor mostra o la stampante produce:

> ALIMENTAZIONE MAGNETI INTERROTTA  
> SETTORI DANNEGGIATI: TF-2, TF-5, TF-7  
> SOSTITUIRE ESCLUSIVAMENTE I MODULI INDICATI

### Risultato e collegamento

L'accesso alla sala del nocciolo è disponibile e la squadra conosce i tre settori da riparare. «TF» identifica i settori delle bobine toroidali nell'interfaccia del gioco.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Tastiera numerica a matrice | 1 | Membrana 3 × 4, 12 tasti | Distinta dalla tastiera della porta |
| Portafusibile e fusibile di scena | 1 set | Portafusibile 5 × 20 mm e fusibile in vetro 5 × 20 mm, 250 mA | Usati come contatto a bassissima tensione; non protegge l'alimentazione principale |
| Resistenza di pull-up | 1 | 10 kΩ, ¼ W | Rilevazione del contatto del fusibile |

## 6. Sostituzione dei moduli

### Narrazione e obiettivo

Tre settori del toroide sono danneggiati. La squadra deve sostituire i relativi moduli prima di riallineare le bobine.

### Scenario e indizi

Il modello del toroide ha otto alloggiamenti numerati **TF-1–TF-8**. Un vano accessibile contiene dieci moduli di ricambio, etichettati; soltanto quelli per TF-2, TF-5 e TF-7 sono compatibili con i settori indicati dalla diagnosi. Le etichette sui moduli e sugli alloggiamenti permettono di abbinarli.

### Ragionamento e soluzione

Si rimuovono i tre moduli danneggiati e si inseriscono i ricambi **TF-2**, **TF-5** e **TF-7** nei rispettivi alloggiamenti, lasciando intatti gli altri.

### Meccanismo fisico e riscontro

Ogni ricambio corretto può contenere una resistenza identificativa letta dall'alloggiamento tramite un ingresso analogico. Un LED verde conferma ogni sostituzione corretta. Quando tutte e tre sono confermate, la squadra può aprire manualmente la copertura del toroide e il monitor mostra **MODULI TF RIPRISTINATI**. Il sistema tenta il riavvio e richiede il riallineamento.

### Risultato e collegamento

Le bobine sono riparate, ma il campo non è ancora equilibrato. Si passa ai regolatori della sala di controllo.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Contatti per moduli estraibili | 6 | Pogo pin a molla, passo da definire | Due contatti per ciascuno dei tre alloggiamenti riparabili |
| Resistenze identificative | 10 | 1%: 1 kΩ, 2,2 kΩ, 4,7 kΩ, 6,8 kΩ, 10 kΩ, 15 kΩ, 22 kΩ, 33 kΩ, 47 kΩ, 68 kΩ; una per valore | Una in ciascun ricambio; 1 kΩ per TF-2, 2,2 kΩ per TF-5, 4,7 kΩ per TF-7 |
| Resistenze per partitori | 3 | 10 kΩ, 1%, ¼ W | Una per alloggiamento, verso 3,3 V |
| Condensatori per ingressi analogici | 3 | 100 nF ceramico | Uno per ingresso, verso massa |
| LED verde | 3 | LED 3 mm | Uno per settore riparabile |
| Resistenza per LED | 3 | 1 kΩ, ¼ W | Una per LED |
| Microinterruttore a leva | 1 | KW11-3Z, normalmente aperto | La copertura viene liberata da un fermo manuale dopo la conferma elettronica |

## 7. Riallineamento delle bobine

### Narrazione e obiettivo

Il tentativo di riavvio si arresta: il campo magnetico è sbilanciato. Occorre rendere uguale l'intensità delle bobine opposte.

### Scenario e indizi

Le otto bobine numerate del toroide hanno ciascuna un display. Un documento spiega che le bobine diametralmente opposte devono essere bilanciate. Una cartella aggiunge: «Ogni regolatore agisce su una coppia opposta: aumentando un valore, l'altro diminuisce della stessa quantità».

Quattro manopole nella sala di controllo sono etichettate con le coppie: **TF-A: 1–5**, **TF-B: 2–6**, **TF-C: 3–7**, **TF-D: 4–8**. I valori iniziali sono:

| Coppia | Valori iniziali |
|---|---|
| 1–5 | 32 / 36 |
| 2–6 | 42 / 38 |
| 3–7 | 28 / 24 |
| 4–8 | 35 / 39 |

### Ragionamento e soluzione

Per ogni coppia la somma resta costante: i giocatori regolano le manopole fino a dividere la somma in due valori uguali.

| Coppia | Valori equilibrati |
|---|---|
| 1–5 | 34 / 34 |
| 2–6 | 40 / 40 |
| 3–7 | 26 / 26 |
| 4–8 | 37 / 37 |

### Meccanismo fisico e riscontro

Ogni manopola aggiorna contemporaneamente i due display della coppia, aumentando un valore e diminuendo l'altro. Il sistema verifica tutte e quattro le coppie; a equilibrio raggiunto mostra **CAMPO MAGNETICO STABILE**.

### Risultato e collegamento

Il confinamento magnetico è disponibile. Il monitor segnala però **CAMERA A VUOTO NON SIGILLATA** e impedisce lo scarico.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Potenziometro lineare | 4 | B10K, 10 kΩ lineare | Uno per coppia di bobine; manopola stampata |
| Display numerico | 8 | TM1637, 4 cifre a 7 segmenti, modulo utilizzabile a 3,3 V | Uno per bobina; visualizza valori interi |
| Condensatori per ingressi analogici | 4 | 100 nF ceramico | Uno per cursore, verso massa |

## 8. Chiusura della camera a vuoto

### Narrazione e obiettivo

Per abilitare lo scarico bisogna chiudere in sequenza le quattro sezioni della camera.

### Scenario e indizi

Sul modello ci sono quattro condotti con ghiere rotanti, contrassegnate da cerchio, triangolo, quadrato ed esagono. Una targhetta dice: «Chiudere le sezioni in ordine crescente di numero di lati». Uno schema associa al cerchio 0 lati, al triangolo 3, al quadrato 4 e all'esagono 6.

### Ragionamento e soluzione

Seguendo i valori indicati si chiudono le ghiere nell'ordine **cerchio → triangolo → quadrato → esagono**.

### Meccanismo fisico e riscontro

Ogni ghiera ha una posizione aperta e una chiusa rilevata da un microinterruttore. Il sistema registra l'ordine di chiusura; un errore richiede di riaprire tutte le ghiere e ripartire. Alla sequenza corretta si accende una striscia verde e compare **CAMERA A VUOTO SIGILLATA**.

### Risultato e collegamento

La camera è pronta per lo scarico, ma il sistema automatico di instradamento resta guasto.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Microinterruttore a leva | 4 | KW11-3Z, normalmente aperto | Uno per ghiera, azionato da camma stampata |
| LED verde | 1 | LED 3 mm | Illumina la striscia di tenuta tramite guida di luce stampata |
| Resistenza per LED | 1 | 1 kΩ, ¼ W | Per il LED di tenuta |

## 9. Instradamento del plasma

### Narrazione e obiettivo

La squadra deve collegare manualmente il toroide al serbatoio di contenimento senza attraversare i circuiti criogenici.

### Scenario e indizi

Un pannello rappresenta sei condotti e quattro valvole deviatrici a due posizioni. Una mappa tecnica distingue linee rosse per il plasma, blu per il raffreddamento e gialle per l'alimentazione. Una nota avverte: «Durante lo scarico di emergenza il plasma non deve attraversare linee criogeniche». La mappa deve mostrare chiaramente le diramazioni e identificare i condotti **P3**, **P5** e **P6**.

### Ragionamento e soluzione

Seguendo il percorso rosso continuo sulla mappa, i giocatori impostano le quattro leve per collegare **toroide → P3 → P5 → P6 → serbatoio**. La posizione esatta di ogni leva dipende dal disegno definitivo della mappa e deve essere ricavabile da esso.

### Meccanismo fisico e riscontro

Una possibile realizzazione prevede una piccola sfera metallica che percorre i condotti sotto una copertura: le leve aprono o deviano fisicamente il passaggio. Quando raggiunge il serbatoio, un sensore rileva la sfera e accende un LED blu. In alternativa, i sensori delle quattro leve possono verificare la configurazione senza utilizzare la sfera.

### Risultato e collegamento

Il monitor mostra **SERBATOIO DI CONTENIMENTO COLLEGATO** e abilita la procedura finale.


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Interruttore a levetta | 4 | Mini SPDT ON-ON, a scatto | Una leva per deviatore; due posizioni rilevate |
| LED blu | 1 | LED 3 mm | Serbatoio collegato |
| Resistenza per LED | 1 | 1 kΩ, ¼ W | Per il LED blu |

## 10. Scarico del plasma

### Narrazione e obiettivo

Il campo è stabile, la camera è sigillata e il serbatoio è collegato. Resta da eseguire lo spegnimento manuale nell'ordine previsto.

### Scenario e indizi

Il pannello finale presenta tre leve: **INIETTORI**, **MAGNETI** e **SCARICO**. Una placca riporta:

1. Disattivare gli iniettori di combustibile.
2. Mantenere il confinamento magnetico.
3. Aprire il canale di scarico.

### Ragionamento e soluzione

I giocatori impostano, nell'ordine, **INIETTORI → OFF**, **MAGNETI → ON** e **SCARICO → OPEN**. Le leve devono partire in posizioni che consentano di registrare queste tre azioni.

### Meccanismo fisico e riscontro

Il sistema verifica sia l'ordine delle azioni sia lo stato finale delle leve. Se la sequenza è errata, segnala errore e richiede di riportare le leve alle posizioni iniziali. Con la sequenza corretta, l'anello rosso nel toroide si affievolisce, una barra luminosa scorre verso il serbatoio e quest'ultimo si illumina di blu.

### Risultato e collegamento

Si accende una luce verde e il monitor conclude:

> PLASMA SCARICATO  
> TEMPERATURA CAMERA STABILIZZATA  
> MAGNETI SUPERCONDUTTORI OPERATIVI  
> RISCHIO ESPLOSIONE ELIMINATO  
> CENTRALE NUCLEARE DOLOMITI SALVATA


### Componenti hardware

| Componente | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Interruttore a levetta | 3 | Mini SPDT ON-ON, a scatto | Iniettori, magneti e scarico |
| LED indirizzabili | 24 | WS2812B, 5 V, striscia tagliabile | 12 toroide, 8 percorso, 3 serbatoio, 1 luce finale |
| Resistenza sulla linea dati LED | 1 | 330 Ω, ¼ W | Vicina all'ingresso della striscia |
| Condensatore elettrolitico | 1 | 1000 µF, almeno 10 V | Fra 5 V e GND vicino alla striscia |

## Dettagli da definire in fase di realizzazione

- Assegnare un uso alle due chiavi e collocare il quarto frammento della tavola periodica, oppure rimuoverli dagli oggetti di scena.
- Disegnare la mappa dell'enigma 9 e stabilire la posizione corretta delle quattro leve; verificare che il percorso sia univoco e realizzabile con il meccanismo scelto. L'idea alternativa di un percorso fisico o di collegamenti senza sovrapposizioni resta da progettare.
- Scegliere come rilevare le valvole criogeniche e i moduli, e se usare stampante, messaggi a schermo o entrambi. Il potenziometro con indicazione di pressione è un'idea da valutare per le valvole.
- Definire ubicazione e apertura effettiva della sala del nocciolo, la copertura sbloccata nell'enigma 6 e le posizioni iniziali delle leve finali.

## Architettura e componenti condivisi

La distinta seguente descrive una **configurazione di riferimento**, da verificare sul modello stampato. La console di gioco dispone già di ESP32-S3 N16R8, display TFT da 2,8 pollici, microSD, altoparlante e amplificatore audio: la stanza li utilizza tramite le interfacce della console e non deve acquistarli o duplicarli. Il formato, la gestione dei file audio e le interfacce effettivamente disponibili dipendono dalla console. Le porte vengono sbloccate da microservi e aperte a mano: non sono necessarie elettroserrature o motori per muovere le ante. La porta del nocciolo è considerata aperta. Per l'enigma 9 si sceglie la lettura delle quattro leve, senza sfera o sensore di arrivo. La copertura dell'enigma 6 si apre a mano dopo la conferma; il suo microinterruttore ne rileva l'apertura e il fermo meccanico non è comandato elettricamente. Il display e il sistema audio della console riproducono messaggi e suoni e sostituiscono rispettivamente il monitor e il riproduttore audio dedicati; il display sostituisce la stampante; la seconda tastiera è dedicata alla console. Badge, chiavi, documenti e pezzi 3D sono oggetti di scena.

| Componente condiviso | Quantità | Identificativo / specifica | Utilizzo |
|---|---:|---|---|
| Espansore ingressi/uscite | 4 | MCP23017, I²C, 16 GPIO, 3,3 V | Pulsanti, interruttori, LED e linee dei display; indirizzi I²C distinti |
| Convertitore analogico | 2 | ADS1115, I²C, 4 canali, 3,3 V | Sette ingressi analogici: tre moduli e quattro potenziometri; indirizzi distinti |
| Driver servo | 1 | PCA9685, I²C, 16 canali | Quattro servi, logica a 3,3 V e alimentazione servo separata a 5 V |
| Adattatore di livello | 1 | 74AHCT125, alimentato a 5 V | Porta il segnale dati WS2812B da 3,3 V a 5 V |
| Protezione alimentazione | 1 set | Portafusibile in linea + fusibile 5 A, adeguati all'alimentatore | Protezione del ramo 5 V; scegliere cavi e portafusibile con portata adeguata |
| Interruttore generale | 1 | SPST, almeno 5 V CC / 8 A | Sezionamento del ramo a bassa tensione |
| Condensatore elettrolitico | 1 | 1000 µF, almeno 10 V | Vicino alla distribuzione dei servi |
| Resistenze pull-up I²C | 2 | 4,7 kΩ, ¼ W | Una per SDA e SCL a 3,3 V; verificare e rimuovere eventuali pull-up duplicati sui moduli |
| Basette millefori | 2 | PCB millefori, dimensioni da definire | Distribuzione e circuiti di interfaccia |
| Morsetti e connettori | 1 kit | Morsetti a vite, JST-XH 2/3/4 poli e relativi terminali | Connessioni separabili per sensori e attuatori |
| Cavi | 1 kit | Filo flessibile, sezioni adatte alle correnti; cavetti segnale | Cablaggio, con alimentazione LED e servi dimensionata alla corrente |
| Materiale di assemblaggio | 1 kit | Stagno, guaina termorestringente, fascette, viti M2/M3 e inserti filettati | Montaggio e manutenzione |
| Materiale di stampa | 1 lotto | PLA/PETG, quantità da stimare sui modelli | Struttura, pulsanti, manopole, supporti e guide luce |

### Collegamenti e verifica del prototipo

- Alimentare le uscite di segnale di ESP32-S3, MCP23017, ADS1115, RC522 e TFT a **3,3 V**; usare solo moduli TM1637 effettivamente compatibili con 3,3 V. SDA e SCL devono avere pull-up a 3,3 V, non a 5 V. Non collegare segnali a 5 V agli ingressi ESP32-S3.
- Servi e LED sono alimentati dal ramo 5 V, non dai GPIO o dal regolatore della scheda; tutte le masse sono comuni. Il PCA9685 comanda i servi, senza portarvi la loro corrente attraverso la logica. Il 74AHCT125 adatta il segnale dati della striscia; fissare gli ingressi inutilizzati a un livello definito.
- Per ogni alloggiamento dell'enigma 6, collegare un contatto del modulo a massa e l'altro all'ingresso ADS1115; una resistenza da 10 kΩ collega l'ingresso a 3,3 V. L'alloggiamento vuoto legge circa 3,3 V. Calibrare soglie e tolleranze per distinguere tutti i valori, incluso circuito aperto; i moduli danneggiati iniziali non devono contenere la resistenza del ricambio corretto.
- Il dimensionamento prevede al massimo 24 LED a piena luminosità e quattro servi: limitare la luminosità e non azionare i servi contemporaneamente. Misurare corrente di spunto e caduta di tensione, quindi confermare alimentatore, protezione e cablaggio. Il fusibile scenico dell'enigma 5 è inserito solo in un circuito di rilevazione a 3,3 V limitato dalla resistenza.
- La disposizione fisica e il numero di GPIO disponibili dipendono dall'hardware della console. Pianificare indirizzi I²C, pin SPI/UART, alimentazione e cablaggio prima dell'acquisto; evitare conflitti con periferiche esistenti e pin di avvio. I display TM1637 richiedono due linee ciascuno e aggiornamento software tramite espansori. Verificare nel prototipo tempi di aggiornamento, leggibilità e compatibilità dei moduli.
- Gli interruttori usano pull-up a 3,3 V e contatto verso massa; prevedere antirimbalzo software. Le tastiere sono matrici da 7 linee ciascuna. Meccanismi, ingombri, corse dei servi, dimensioni dei cappucci e visibilità dei display devono essere provati nella scala 1:10.

## Resoconto totale dei componenti

Quantità da aggiungere per **una stanza**, senza scorte. ESP32-S3 N16R8, TFT 2,8 pollici, microSD, altoparlante e amplificatore sono già nella console e non sono conteggiati; i componenti condivisi dalla stanza sono contati una sola volta e i pezzi opzionali non sono inclusi.

| Componente | Identificativo / specifica | Totale | Destinazione |
|---|---|---:|---|
| Espansore GPIO | MCP23017 | 4 | Condivisi |
| ADC | ADS1115 | 2 | Enigmi 6–7 |
| Driver servo | PCA9685 | 1 | Enigmi 1–4 |
| Microservo | SG90, 5 V | 4 | Enigmi 1–4 |
| Pulsante momentaneo | Tactile 6 × 6 mm | 9 | Enigmi 1, 4 |
| Tastiera numerica | Matrice a membrana 3 × 4 | 2 | Enigmi 2, 5 |
| Microinterruttore | KW11-3Z | 9 | Enigmi 3, 6, 8 |
| Interruttore a levetta | Mini SPDT ON-ON | 7 | Enigmi 9, 10 |
| Potenziometro | B10K lineare | 4 | Enigma 7 |
| Lettore RFID | RC522 / MFRC522 | 1 | Enigma 3 |
| Badge RFID | MIFARE Classic 1K | 1 | Enigmi 1, 3 |
| Display numerico | TM1637, 4 cifre, 3,3 V | 8 | Enigma 7 |
| LED verde | 3 mm | 5 | Enigmi 1, 6, 8 |
| LED rosso | 3 mm | 3 | Enigmi 1, 3 |
| LED blu | 3 mm | 1 | Enigma 9 |
| LED indirizzabili | WS2812B, 5 V | 24 | Enigma 10 |
| Resistenza LED | 1 kΩ, ¼ W | 9 | Enigmi 1, 3, 6, 8, 9 |
| Resistenza pull-up / partitore | 10 kΩ, ¼ W; 1% per partitori | 5 | Enigmi 5, 6 e CS del lettore RFID |
| Resistenze identificative | 1 kΩ–68 kΩ, valori indicati nell'enigma 6, 1% | 10 | Enigma 6 |
| Resistenza pull-up I²C | 4,7 kΩ, ¼ W | 2 | Condivisa |
| Resistenza dati LED | 330 Ω, ¼ W | 1 | Enigma 10 |
| Condensatore ceramico | 100 nF | 8 | Enigmi 6, 7 e adattatore di livello |
| Condensatore elettrolitico | 1000 µF, almeno 10 V | 2 | LED e servi |
| Contatto a molla | Pogo pin | 6 | Enigma 6 |
| Portafusibile e fusibile scenico | 5 × 20 mm, 250 mA | 1 set | Enigma 5 |
| Adattatore di livello | 74AHCT125 | 1 | LED WS2812B |
| Portafusibile e fusibile di protezione | In linea, 5 A | 1 set | Alimentazione |
| Interruttore generale | SPST, almeno 5 V CC / 8 A | 1 | Alimentazione |
| Basetta millefori | Dimensioni da definire | 2 | Cablaggio |
| Connettori e morsetti | JST-XH / morsetti a vite | 1 kit | Cablaggio |
| Cavi elettrici | Flessibili, sezioni adeguate | 1 kit | Cablaggio |
| Materiale di assemblaggio | Stagno, guaina, fascette, viti e inserti | 1 kit | Montaggio |
| Filamento | PLA/PETG | 1 lotto | Parti stampate |

Fonti tecniche: [Espressif — GPIO e pin disponibili](https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32s3/api-reference/peripherals/gpio.html), [Espressif — indicazioni per GPIO e ADC](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32s3/schematic-checklist.html).

## Proposta di collegamento alla console

Questo è uno schema funzionale ipotetico, non un cablaggio collaudato. Usa soltanto i pin indicati come disponibili e SPI condiviso; i GPIO degli espansori sono assegnati tutti, senza margine. L'alimentazione esterna a 5 V / 8 A è già disponibile e non è conteggiata come componente della stanza. I segnali e le alimentazioni devono essere compatibili con la console effettivamente montata.

### Pin ESP32-S3

| Pin disponibile | Collegamento proposto |
|---|---|
| GPIO1 | SDA del bus I²C dedicato alla stanza |
| GPIO2 | SCL dello stesso bus I²C |
| GPIO11 condiviso | MOSI / SDA del lettore RC522, in parallelo al bus SPI del TFT |
| GPIO12 condiviso | SCK del lettore RC522 |
| GPIO13 condiviso | MISO del lettore RC522 |
| GPIO41 | SS / SDA di selezione SPI del lettore RC522, distinto da tutti gli altri CS |
| GPIO40 | RST del lettore RC522 |
| GPIO39 | Dato dei LED WS2812B, tramite 74AHCT125 e resistenza da 330 Ω |
| GPIO42 | Rilevazione del fusibile scenico dell'enigma 5 |
| 3V3 e GND | Logica a 3,3 V e massa comune |

Attenzione: sul modulo RC522 il pin marcato **SDA** è il chip select SPI; non va collegato alla linea SDA del bus I²C. Il suo pin IRQ può restare scollegato. Configurare GPIO41 come uscita alta prima di inizializzare il bus; usare una pull-up da 10 kΩ a 3,3 V sul CS per mantenerlo disattivo durante il reset.

I GPIO6 e 7 sono indicati come disponibili nella mappa fornita, ma in `BoardConfig.h` sono assegnati a IRQ e CS del touch; GPIO3 vi è assegnato al controllo di alimentazione SD, GPIO14 a SDA I²C e GPIO48 al LED integrato. Questa proposta non li usa. GPIO0, 3 e 45 hanno vincoli di avvio; GPIO19 è associato a USB: sono lasciati liberi. Verificare il cablaggio reale prima di usare qualsiasi pin, compresi quelli indicati come liberi.

Il progetto usa GPIO11/12/13 per TFT e touch, mentre la microSD ha un bus SPI distinto. L'RC522 deve condividere il controller e il bus del TFT con transazioni coordinate, CS indipendenti e frequenza compatibile. L'attuale `SPIBusManager` gestisce il bus della microSD: non garantisce da solo la sincronizzazione con LovyanGFX sul bus TFT. Servono integrazione firmware e verifica che le periferiche rilascino MISO quando non selezionate; fino a queste verifiche il collegamento resta teorico.

### Bus I²C della stanza

Collegare SDA e SCL in parallelo a GPIO1 e GPIO2, con pull-up a 3,3 V sulle due linee. Alimentare i circuiti logici a 3,3 V e tutte le masse in comune. Verificare indirizzi, pull-up già montate e capacità del bus; partire a 100 kHz e misurare l'integrità dei segnali con i cavi reali. Il bus è separato da quello eventualmente configurato nella console; il firmware deve inizializzarlo e gestirlo esplicitamente.

| Dispositivo | Indirizzo I²C a 7 bit | Impostazione e utilizzo |
|---|---|---|
| MCP23017 A | `0x20` | A2/A1/A0 = 0/0/0 |
| MCP23017 B | `0x21` | A2/A1/A0 = 0/0/1 |
| MCP23017 C | `0x22` | A2/A1/A0 = 0/1/0 |
| MCP23017 D | `0x23` | A2/A1/A0 = 0/1/1 |
| ADS1115 A | `0x48` | ADDR a GND; tre moduli e un potenziometro |
| ADS1115 B | `0x49` | ADDR a VDD; tre potenziometri |
| PCA9685 | `0x40` | Indirizzo base; quattro uscite servo |

Per gli MCP: VDD a 3,3 V, VSS a GND, RESET mantenuto alto a 3,3 V; INTA e INTB scollegati perché è previsto il polling. I bit A0–A2 vanno fissati a GND o 3,3 V come in tabella. Per gli ADS1115, ALERT/RDY non è usato. Sul PCA9685: VCC logica a 3,3 V, V+ servo a 5 V e GND comune; OE a GND per abilitare le uscite. Verificare le serigrafie e i circuiti dei breakout scelti.

### Assegnazione completa degli espansori

Per gli ingressi singoli usare pull-up interna MCP e contatto verso GND; il valore attivo è basso. Le tastiere richiedono scansione riga/colonna con righe pilotate una alla volta e colonne con pull-up; non collegare permanentemente a massa le righe. I LED sono collegati da 3,3 V a resistenza da 1 kΩ, anodo LED, catodo verso GPIO: il LED si accende portando l'uscita bassa.

| Espansore | Pin | Collegamento |
|---|---|---|
| A | GPA0–GPA3 | Quattro pulsanti ascensore, enigma 1 |
| A | GPA4–GPA7, GPB0 | Cinque pulsanti armadio, enigma 4 |
| A | GPB1–GPB4 | Quattro valvole, enigma 3 |
| A | GPB5, GPB6 | LED verde e rosso ascensore |
| A | GPB7 | Primo LED rosso allarme criostato |
| B | GPA0–GPA6 | Tastiera porta: quattro righe, tre colonne, enigma 2 |
| B | GPA7, GPB0–GPB5 | Tastiera alimentazione: quattro righe, tre colonne, enigma 5 |
| B | GPB6 | Secondo LED rosso allarme criostato |
| B | GPB7 | LED verde settore TF-2 |
| C | GPA0–GPA3 | Quattro ghiere, enigma 8 |
| C | GPA4–GPA7 | Quattro deviatori, enigma 9 |
| C | GPB0–GPB2 | Tre leve finali, enigma 10 |
| C | GPB3 | Microinterruttore copertura toroide, enigma 6 |
| C | GPB4, GPB5 | LED verdi settori TF-5 e TF-7 |
| C | GPB6, GPB7 | LED verde tenuta camera e LED blu serbatoio |
| D | GPA0/GPA1 … GPA6/GPA7 | CLK/DIO dei display bobine 1–4, una coppia per display |
| D | GPB0/GPB1 … GPB6/GPB7 | CLK/DIO dei display bobine 5–8, una coppia per display |

I moduli TM1637 non usano il protocollo I²C. CLK si pilota da uscita; DIO deve poter essere rilasciato come ingresso per l'ACK e non va tenuto attivamente alto contro il display. Alimentare i moduli a 3,3 V e verificare che eventuali pull-up di CLK/DIO siano a 3,3 V. Occorre un driver TM1637 adattato agli accessi MCP23017 e testarne velocità e comportamento; una libreria che presuppone GPIO diretti non è sufficiente. Con quattro espansori non rimangono GPIO liberi: per eventuali aggiunte serve un altro espansore o una nuova ripartizione.

### Analogici, servi e alimentazione

| Collegamento | Destinazione |
|---|---|
| ADS1115 A, AIN0–AIN2 | Partitori dei moduli TF-2, TF-5, TF-7: ingresso a 3,3 V tramite 10 kΩ, modulo verso GND, 100 nF dall'ingresso a GND |
| ADS1115 A, AIN3 | Cursore potenziometro TF-A |
| ADS1115 B, AIN0–AIN2 | Cursori potenziometri TF-B, TF-C, TF-D; AIN3 libero |
| Potenziometri | Estremi a 3,3 V e GND; ogni cursore al rispettivo ingresso e a 100 nF verso GND |
| PCA9685, canali 0–3 | Segnale rispettivamente ai servi ascensore, porta controllo, vano e armadio; impostare circa 50 Hz |
| GPIO42 | Pull-up 10 kΩ a 3,3 V; fusibile scenico inserito fra GPIO42 e GND, chiuso = livello basso |
| GPIO39 → 74AHCT125 → 330 Ω → DIN WS2812B | Adattamento dati a 5 V; alimentare 74AHCT125 a 5 V, collegare OE attivo basso a GND, fissare ingressi inutilizzati e aggiungere 100 nF fra VCC e GND vicino al chip |

Distribuire i 5 V esterni su un ramo protetto per la stanza, con masse comuni alla console: servi, WS2812B e 74AHCT125 usano 5 V; logica, sensori e pull-up usano 3,3 V dalla console. I 5 V non vanno sui GPIO o sul pin 3V3. Prima di collegare l'alimentazione esterna al 5Vi della console verificare schema della scheda, percorso di alimentazione e possibile ritorno verso USB: non collegare alla cieca due sorgenti. La protezione deve essere dimensionata per i rami e i cavi effettivi, considerando anche gli assorbimenti della console.

La resistenza da 10 kΩ per il CS RC522 e il condensatore da 100 nF per il 74AHCT125 sono inclusi nel totale. Il regolatore 3,3 V della console deve poter alimentare anche tutti i moduli aggiuntivi; verificare assorbimento totale e cadute di tensione. Verificare che i servi scelti riconoscano il segnale di controllo a 3,3 V del PCA9685; altrimenti usare un adattatore di livello appropriato. Nessun collegamento fisico o modifica firmware è stato eseguito.
