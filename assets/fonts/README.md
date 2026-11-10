# Escaperiom Display

Font e generatore forniti dall'utente il 4 novembre 2026, basati sul logo
Escaperiom. Il generatore adattato e' in `scripts/fonts/make_escaperiom_font.py`.
La variante 1.100 conserva la geometria, alleggerisce i tratti da 12 a 10 unita',
riduce la larghezza all'86%, corregge i side bearing e aggiunge accenti Latin-1,
apostrofi tipografici, trattini e simbolo euro. Le minuscole usano forme maiuscole;
la `o` minuscola conserva la variante senza nucleo, come nel font originale.

Il font e' destinato ai titoli. Per testi lunghi, pulsanti e timer restano i font
di lettura esistenti. Il logo vettoriale conserva la propria geometria originale.

TTF e WOFF2 sono qui; la copia web e' in `frontend/src/assets/fonts`. I bitmap
LVGL 18/22/26 px, a 4 bpp e senza decompressione, sono in `include/fonts`.
Sono inclusi nel firmware: non richiedono accessi SD o rasterizzazione TTF sull'ESP32.

Per rigenerarli dalla radice del progetto:

```powershell
python -m pip install -r scripts/fonts/requirements.txt
npm ci --prefix scripts/fonts
python scripts/fonts/build_fonts.py
```

Le normali build PlatformIO usano i bitmap gia' versionati e non eseguono questi
strumenti. Per il portale eseguire anche `npm run build` in `frontend`, poi
copiare `sd/.core/portal` sulla scheda fisica.
