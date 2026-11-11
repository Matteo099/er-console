# Diagnostica su dispositivo

Questi strumenti comunicano con l'ESP32 tramite seriale: richiedono `pyserial`, un firmware debug con `UI_DIAGNOSTICS_ENABLED=1` e una porta disponibile. Non sono eseguiti dalla CI. Alcune prove interagiscono con impostazioni o giochi: consultare la documentazione della prova prima di avviarla.

Dalla radice del repository, ad esempio:

```sh
python scripts/diagnostics/ui_capture.py --port COM3 --snapshot schermata
python scripts/diagnostics/ui_settings.py --help
```

- `ui_capture.py` acquisisce schermate e albero UI; `ui_stability.py` fornisce anche funzioni seriali condivise.
- `ui_settings.py`, `ui_ingame_settings.py`, `ui_network.py`, `ui_scroll.py` e `ui_volume_feedback.py` verificano controlli e interazioni.
- `ui_game_assets.py`, `ui_game_translations.py` e `ui_statistics.py` verificano risorse, traduzioni e statistiche dei giochi.
- `ui_performance.py` misura le scene; `ui_ota.py` verifica gli aggiornamenti.
- `audio_check.py`, `audio_preload.py` e `audio_preload_game.py` verificano mixer e precaricamento audio.

Per i prerequisiti e i dettagli: [riferimenti e diagnostica](../../wiki/reference/Index.md) e [OTA](../../wiki/OTA.md).
