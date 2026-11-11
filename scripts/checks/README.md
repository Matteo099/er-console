# Controlli delle traduzioni

Dalla radice del repository:

```sh
python scripts/checks/check_translations.py
python scripts/checks/check_game_translations.py
```

Il primo controlla il catalogo dell'interfaccia firmware; il secondo verifica metadati e messaggi Lua dei giochi. Entrambi controllano le traduzioni e le firme di formattazione e sono eseguiti dalla CI.
