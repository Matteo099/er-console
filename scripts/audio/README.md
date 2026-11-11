# Strumenti audio

Eseguire i comandi dalla radice del repository.

- `standardize_audio.py` verifica o converte i file in WAV PCM mono a 22.050 Hz, 16 bit. La conversione usa FFmpeg quando necessario e salva in una cartella separata.
- `click.py`, `countdown.py`, `defeat.py`, `main_theme.py` e `victory.py` generano il rispettivo WAV nella cartella di lavoro corrente; i WAV presenti qui sono esempi generati.

```sh
python scripts/audio/standardize_audio.py --check sd
python scripts/audio/standardize_audio.py sd --output audio-standardized
python scripts/audio/standardize_audio.py --help
```

Per formato, opzioni e test: [Audio](../../wiki/reference/AUDIO.md).
