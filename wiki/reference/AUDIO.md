# Audio: sorgenti, loop e preparazione dei file

Il mixer software supporta **otto sorgenti simultanee**, anche nello stesso
gruppo e anche sullo stesso WAV, con cursori indipendenti. I beep occupano una
sorgente come i file. Non interrompe automaticamente un'altra sorgente quando
gli slot sono pieni: la richiesta viene rifiutata.

Il MAX98357 riceve un unico flusso I2S mono, PCM signed 16 bit, a 22050 Hz.
I volumi indipendenti sono possibili: il mixer applica il guadagno a ciascuna
sorgente prima di sommare i campioni. Non occorre cambiare hardware.

## API Lua

```lua
-- Caricare durante la preparazione della partita, prima della riproduzione.
audio.preload("/games/demo/music.wav")
audio.preload("/games/demo/ambience.wav")

local music = assert(audio.play("/games/demo/music.wav", {
    loop = true, channel = "music", volume = 35
}))
local ambience = assert(audio.play("/games/demo/ambience.wav", {
    loop = true, channel = "music", volume = 20
}))
local effect = assert(audio.play("/games/demo/hint.wav", false, "effects", 80))

audio.setSourceVolume(ambience, 10)
local level = audio.getSourceVolume(ambience)
local active = audio.isPlaying(music)
audio.stop(effect)       -- solo questa sorgente
audio.stop("music")      -- tutte le sorgenti del gruppo musica
audio.stop()             -- tutte le sorgenti, inclusi i beep
```

| Chiamata | Risultato |
| --- | --- |
| `audio.play(path, loop, channel, volume)` | ID positivo, oppure `nil, errore` se coda/slot pieni o percorso non valido |
| `audio.play(path, {loop=..., channel=..., volume=...})` | Stesso risultato, con opzioni nominate |
| `audio.stop(id)` | `true` se la sorgente era ancora registrata, altrimenti `false` |
| `audio.stop("music"/"effects"/"all")` | Interrompe il gruppo, comprese le richieste gia' in coda |
| `audio.setSourceVolume(id, 0..100)` | `true` per un ID ancora registrato, altrimenti `false` |
| `audio.getSourceVolume(id)` | Volume, oppure `nil` per un ID terminato/non valido |
| `audio.isPlaying(id)` | `true` mentre la richiesta e' in coda, in riproduzione o sospesa da un gruppo disattivato |
| `audio.preload(path)` | Accettazione della richiesta asincrona di caricamento in cache |

I parametri opzionali mantengono la compatibilita': loop `false`, volume `100`,
gruppo `music` per i loop e `effects` per i suoni singoli. Il caricamento dei WAV
avviene nel task audio: un ID indica accettazione, non certifica la lettura del
file. Errori SD/formato vengono registrati nei log e liberano la sorgente;
`isPlaying` diventa `false` anche dopo EOF o stop. Un vecchio ID non puo'
controllare una nuova sorgente che riutilizza lo stesso slot.

`loop=true` riavvolge il WAV senza limite di ripetizioni fino a stop, anche per
il gruppo effetti. Il file deve avere estremi adatti al loop: non vengono
aggiunti dissolvenze o crossfade automatici.

## Volumi e comportamento dei gruppi

Il segnale risultante applica tre livelli: volume della sorgente, volume
Musica/Effetti delle Impostazioni e master `audio.setVolume(0..100)`.
Le preferenze delle Impostazioni restano indipendenti dai guadagni dei singoli
file. Ad esempio, sorgente al 40% e gruppo al 50% danno il 20% prima del master.

Disattivare Musica sospende le sue sorgenti, conservandone la posizione.
Disattivare Effetti scarta i suoni singoli e sospende i loop, che riprendono
riattivando il gruppo. Il master a zero sospende l'uscita; gli effetti singoli
richiesti mentre sono disattivati vengono scartati. Il volume di una singola
sorgente a zero la lascia invece avanzare in silenzio, fino a EOF o stop.

Le somme vengono saturate a 16 bit per evitare overflow. Per evitare distorsione
quando molti file sono forti contemporaneamente, lasciare margine nei loro
volumi: abbassare solo il master dopo la somma non recupera il clipping.

## Cache e limiti

`AudioSystem` possiede driver, mixer e cache; solo il task audio li utilizza.
I comandi di riproduzione usano la coda, mentre stop e guadagno per sorgente
usano una mailbox protetta: funzionano anche con la coda piena.
Il render PCM non alloca memoria e non accede alla SD.

La cache contiene al massimo otto file e **4 MiB complessivi in PSRAM** su questa
scheda. Senza PSRAM il budget e' il minore tra 128 KiB e un terzo della RAM libera.
Si mantengono in memoria i file completi: non e' uno streaming per brani di
durata arbitraria. A 44100 byte/s, 4 MiB corrispondono a circa 95 secondi totali
di file unici attivi, esclusi header e metadati. Piu' sorgenti sullo stesso file
condividono i dati ma mantengono cursori e guadagni separati.

I file in uso, inclusi i loop sospesi, non vengono eliminati dalla cache.
Le Scene di gioco possono anche prenotare file non ancora in riproduzione:
[precaricamento, tempi e gestione della cache](AUDIO_PRELOAD.md).
Se il budget non basta, la nuova riproduzione fallisce senza rimuovere quelle
attive. I file inattivi possono essere sostituiti. Il caricamento a freddo
resta sincrono nel task audio e puo' interrompere brevemente l'uscita: richiedere
`preload` durante il caricamento della partita, prima di avviare i loop.
`preload` e' asincrono e non garantisce che un file resti in cache dopo ulteriori
caricamenti; usa gli stessi limiti ed espulsioni.

Il decoder accetta WAV PCM 16 bit mono/stereo da 8 a 96 kHz, riducendo lo stereo
a mono e adattando la frequenza. Per una qualita' e un carico prevedibili,
preparare i file nel formato canonico seguente.

## Script indipendente da PlatformIO

`scripts/audio/standardize_audio.py` usa Python 3.9+ e la libreria standard. FFmpeg
serve solo per convertire file non conformi; non e' necessario per validare
o copiare WAV gia' conformi. Non importa PlatformIO e non scarica dipendenze.

| Parametro | Formato canonico |
| --- | --- |
| Contenitore / codec | RIFF WAV, PCM lineare signed little-endian (`pcm_s16le`, format tag 1) |
| Frequenza | 22050 Hz |
| Canali | 1, mono |
| Profondita' | 16 bit |
| Bitrate PCM | 352800 bit/s, 44100 byte/s |
| Allineamento frame | 2 byte |

```powershell
# Controllare ricorsivamente, incluse cartelle nascoste come .core.
python scripts/audio/standardize_audio.py --check sd --report audio-check.json

# Convertire in una directory separata, conservando i percorsi relativi.
python scripts/audio/standardize_audio.py sd --output audio-standardized --report audio-report.json

# Percorso esplicito di FFmpeg, se non presente nel PATH.
python scripts/audio/standardize_audio.py sd --output audio-standardized --ffmpeg 'C:/Tools/ffmpeg.exe'
```

Sono riconosciuti WAV, MP3, FLAC, OGG, OPUS, M4A, AAC, AIF/AIFF, WMA, CAF e W64.
Tutti gli output hanno estensione `.wav`; la conversione puo' essere lossy
per ricampionamento e riduzione a mono. Non viene normalizzato il loudness.
Originali e loro nomi restano intatti. Aggiornare i percorsi negli script Lua
se si passa da un'estensione compressa a `.wav`.

Lo script verifica struttura RIFF, chunk, frame, codec e parametri; rifiuta
output che collidono (`sound.mp3` e `sound.wav`), directory che sovrascrivono
gli originali e file di destinazione esistenti senza `--overwrite`.
Anche con `--overwrite` gli originali sono protetti. Ogni output viene verificato
in un file temporaneo e sostituito solo dopo il successo.

`--max-bytes` imposta il limite del singolo file (default 4194304; `0` controlla
solo il formato). Questo controllo non puo' sapere quali file saranno attivi
contemporaneamente: rispettare anche il budget complessivo della cache.
Exit code: `0` tutto conforme/convertito; `1` almeno un file fallito; `2` errore
di argomenti, percorso o collisione prima della conversione. Una conversione
di gruppo puo' completare alcuni file prima di fallire su un altro; il report
JSON elenca gli esiti.

Copiare i file prodotti sulla microSD conservando i percorsi; l'upload firmware
non trasferisce gli asset. Gli otto file attuali in `sd/.core/audio` sono stati
convertiti e verificati in `.pio/audio-standardized/.core/audio`, senza cambiare
gli originali. L'archivio `.pio/audio-standardized.zip` conserva questa struttura.

## Verifica

```powershell
python -m unittest discover -s scripts/tests -p test_standardize_audio.py -v
```

I 13 test coprono validazione, conversione reale WAV/MP3 con FFmpeg, durata,
collisioni, protezione originali e conservazione della destinazione su errore.
I test di conversione sono saltati se FFmpeg non e' installato.

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`, `ui audio-test` esegue 41 controlli
su campioni PCM, loop, somme, clipping, resampling, controlli con coda piena e
API Lua. `python scripts/diagnostics/audio_check.py` li esegue via COM3 insieme a
`ui selftest`. Non produce suoni e non certifica la qualita' acustica dello
speaker. Ripristinare il flag diagnostico a zero dopo le prove.

Verifiche eseguite il 7 novembre 2026: 41 controlli audio e 30 controlli generali
superati sull'ESP32; 13 test Python superati, incluse le conversioni FFmpeg.
Build finale debug caricata su COM3 con diagnostica disattivata, Wi-Fi attivo
e portale disattivato come nella configurazione dell'utente. Il boot acquisito
per 22 secondi legge il WAV iniziale dalla SD e raggiunge il menu a 1756 ms,
senza errori SD, panic o brownout durante questa acquisizione.

Log: `.pio/audio-check/session.log`, `.pio/audio-final-upload.log`,
`.pio/audio-final-boot.log`. Simboli e binario corrispondenti:
`.pio/audio-symbols/final.elf` e `.pio/audio-symbols/final.bin`.
