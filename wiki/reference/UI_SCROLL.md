# Click e scorrimento

Tutti i pulsanti TFT creati con `Button::create()` installano
`ButtonClickGuard`. Le Scene e le View continuano a ricevere solo
`LV_EVENT_CLICKED`: non occorrono controlli duplicati nelle singole schermate.

Un puntatore deve iniziare una pressione sul pulsante e rilasciarsi al suo
interno. Sono tollerati piccoli movimenti del dito; oltre 8 pixel di distanza
dal punto iniziale il gesto viene annullato. La distanza considera l'intero
gesto, anche quando ogni singolo aggiornamento sposta il puntatore di poco.
Una volta annullato, tornare al punto di partenza non riabilita il click.
Anche perdere la pressione sul pulsante o uno scroll riconosciuto da LVGL
annulla il gesto. Il controllo della distanza copre inoltre i trascinamenti
su un asse non scrollabile o al bordo della lista, dove non avviene uno scroll.

Il filtro viene eseguito prima dei callback del widget e dell'applicazione:
un click rifiutato non esegue l'azione, il suono o il bubbling. Non disabilita
lo scroll dei contenitori, non introduce ritardi e non modifica gli slider.
Una nuova pressione azzera lo stato e consente subito un tap intenzionale.
L'attivazione da tastiera/encoder e i click programmatici espliciti conservano
il comportamento esistente.

Lo stato e' separato per pulsante, non usa `lv_obj_set_user_data()` e viene
liberato nell'evento `DELETE` di LVGL insieme al widget. La gerarchia di
proprieta' degli oggetti e il ciclo Scene/View restano invariati.

## Verifica

Con build debug e `UI_DIAGNOSTICS_ENABLED=1`:

```powershell
python scripts/diagnostics/ui_scroll.py
```

In alternativa inviare `ui scroll-test` dal monitor seriale. La suite crea
un display LVGL isolato e passa sequenze di pressione, coordinate, rilascio e
delta rotella a `lv_indev_read()`. Verifica tap, jitter, scroll verticale e
orizzontale, contenitori annidati, entrambi i bordi, trascinamenti lenti,
uscita/rientro, attraversamento di altri pulsanti, rotellina/trackpad,
pulsanti disabilitati, tastiera e click programmatici. Un pulsante LVGL senza
filtro funge da controllo per riprodurre il click accidentale.

Questi sono test del percorso LVGL comune a touch e mouse; i delta di
rotellina rappresentano anche il percorso scroll del trackpad. Non collegano
fisicamente mouse o trackpad alla console e non verificano driver aggiuntivi.
Le prove non navigano le schermate dell'utente, non cambiano preferenze e
non producono audio. Ripristinare il flag diagnostico a zero dopo le prove.

Verifica del 7 novembre 2026: **23 controlli scroll superati** sull'ESP32 e
**30 controlli generali superati** dopo la suite. La build finale e' caricata
su COM3 con `UI_DIAGNOSTICS_ENABLED=0`; Wi-Fi e portale conservano rispettivamente
i valori `1` e `0`. Nell'acquisizione finale di 22 secondi il menu viene aperto
a 1868 ms, senza panic o riavvii. E' presente un avviso CRC della SD durante
il mount, che poi riesce.

Log locali: `.pio/scroll-check/session.log`, `.pio/scroll-general-check/`,
`.pio/scroll-final-upload.log`, `.pio/scroll-final-boot.log`.
ELF e binario corrispondenti: `.pio/scroll-symbols/final.elf` e `final.bin`.
