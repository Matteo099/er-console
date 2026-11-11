# Test degli strumenti

`test_standardize_audio.py` verifica il controllo e la conversione audio, incluse le protezioni dei file originali. Non viene eseguito dalla pipeline.

Dalla radice del repository:

```sh
python -m unittest discover -s scripts/tests -p 'test_*.py' -v
```

I casi che eseguono conversioni reali richiedono FFmpeg; se non disponibile vengono saltati.
