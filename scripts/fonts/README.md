# Generazione font

`make_escaperiom_font.py` genera il font Escaperiom; `build_fonts.py` genera anche i bitmap LVGL e aggiorna la copia web. Le build normali usano i font già presenti e non richiedono questi strumenti.

Dalla radice del repository:

```sh
python -m pip install -r scripts/fonts/requirements.txt
npm ci --prefix scripts/fonts
python scripts/fonts/build_fonts.py
```

`package.json` e il lockfile fissano la versione del convertitore `lv_font_conv`. Per output e dettagli: [font Escaperiom](../../wiki/reference/Fonts.md).
