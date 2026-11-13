# Diagnostica su dispositivo

Questi strumenti comunicano con l'ESP32 tramite seriale: richiedono `pyserial`, un firmware debug con `UI_DIAGNOSTICS_ENABLED=1` e una porta disponibile. Non sono eseguiti dalla CI. Alcune prove interagiscono con impostazioni o giochi: consultare la documentazione della prova prima di avviarla.

Dalla radice del repository, ad esempio:

```sh
python scripts/diagnostics/ui_capture.py --port COM3 --snapshot schermata
python scripts/diagnostics/ui_settings.py --help
```

- `ui_capture.py` acquisisce schermate e albero UI; `ui_stability.py` fornisce anche funzioni seriali condivise.
- `ui_settings.py`, `ui_ingame_settings.py`, `ui_network.py`, `ui_scroll.py` e `ui_volume_feedback.py` verificano controlli e interazioni.
- `ui_players.py` verifica tap consecutivi, ripetizione ogni 300 ms, annullamento e limiti del selettore giocatori su un display LVGL isolato, senza avviare giochi o modificare statistiche.
- `ui_network_connection.py` verifica associazione Wi-Fi reale, conferma password e avvio senza blocchi; usa un backup NVS delle credenziali durante i casi vuoto/rete assente. Consultare [NETWORK_UI.md](../../wiki/reference/NETWORK_UI.md) prima della prova.
- `ui_config_storage.py` verifica migrazione SD/NVS e persistenza dopo reset, usando file RAM e namespace NVS di prova senza modificare le impostazioni personali. Il confronto delle credenziali resta sul dispositivo; [documentazione](../../wiki/reference/DEVICE_SETTINGS_STORAGE.md).
- `ui_game_assets.py`, `ui_game_translations.py` e `ui_statistics.py` verificano risorse, traduzioni e statistiche dei giochi.
- `ui_performance.py` misura le scene; `ui_ota.py` verifica gli aggiornamenti.
- `audio_check.py`, `audio_preload.py` e `audio_preload_game.py` verificano mixer e precaricamento audio.

Per i prerequisiti e i dettagli: [riferimenti e diagnostica](../../wiki/reference/Index.md) e [OTA](../../wiki/OTA.md).
