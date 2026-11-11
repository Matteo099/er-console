# Hook PlatformIO

Gli script selezionati in [`platformio.ini`](../../platformio.ini) vengono eseguiti automaticamente prima della build, nell'ambiente PlatformIO:

- `sync_display_configs.py`: sincronizza configurazione LVGL e font nelle dipendenze.
- `remove_onelua.py`: rimuove `onelua.c` dalla dipendenza Lua per evitare compilazioni duplicate.
- `git_rev.py`: inserisce metadati di versione e hash della tabella partizioni nel firmware.

`update_user_setup.py` è un vecchio helper per TFT_eSPI: non è attivo in `platformio.ini`. Questi script richiedono l'ambiente di build PlatformIO e non vanno avviati direttamente con Python.
