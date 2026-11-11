# Strumenti CI e release

Dalla radice del repository:

```sh
npm ci --prefix scripts/ci
npm test --prefix scripts/ci
python -m unittest discover -s scripts/ci -p 'test_*.py' -v
python -m pip install -r scripts/ci/requirements.txt
npm ci --prefix frontend
python scripts/ci/build_bundle.py --channel dev
```

- `build_metadata.py` calcola versione, canale e commit; `build_bundle.py` compila e verifica firmware, portale e pacchetti SD.
- `check_commits.mjs` controlla commit e titolo della pull request; `release.mjs` gestisce la pubblicazione tramite Semantic Release.
- `test_bundle.py` e `release.test.mjs` verificano pacchetti e comportamento delle release. I file npm e `requirements.txt` bloccano le dipendenze.

La pubblicazione è eseguita dalla pipeline su `main`. Dettagli e prerequisiti: [CI/CD](../../wiki/CI-CD.md).
