# Okosóra

* **pulzusmérés (MAX30100-M)**
    * a pulzusszenzor jelének periodikus mintavételezése
    * mérési zaj csökkentése digitális szűréssel
    * szívverések felismerése és pulzusszám számítása BPM-ben
    * érvénytelen mérés felismerése, például ha nincs ujj a szenzoron


* **lépésszám mérése háromtengelyes gyorsulásmérővel (x-nucleo shield)**
    * lépések felismerése küszöbérték és időzítési feltételek alapján


* **aktuális idő mérése belső RTC-vel**
    * óra, perc, másodperc és dátum kezelése


* **megjelenítés OLED kijelzőn**
    * aktuális idő, pulzusszám, lépésszám kijelzése
    * pulzusmérés megjelenítése valós-időben grafikusan
    * menü vezérlése nyomógombokkal
        * óra nézet
        * pulzusmérés nézet
        * lépésszámláló nézet
        * statisztikák megjelenítése
        * lépésszámláló manuális nullázása




* **grafikus python program**
    * pontos idő automatikus beállítása
    * mért adatok kiolvasása és megjelenítése
    * napi statisztikák lekérdezése és CSV-exportja
    * a használathoz szükséges egyéb funkciók