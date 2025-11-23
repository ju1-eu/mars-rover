# 1. Batteriepack (2×18650)

Daten SunFounder-Pack: ([SunFounder Dokumentation][1])

* Kapazität: (2 \times 3{,}7,\mathrm{V} \cdot 2000,\mathrm{mAh})
  → Energieinhalt grob
  [
  E \approx 2 \cdot 3{,}7,\mathrm{V} \cdot 2{,}0,\mathrm{Ah} \approx 14{,}8,\mathrm{Wh}
  ]
* Angegebene Laufzeit: ca. 90 min
* Ladeeingang: 5 V / 2 A
* Ausgang: 5 V / 5 A (über Step-Up/Down im Pack)

Wenn dein Robot z. B. im Mittel 10 W zieht, sind 14,8 Wh ≈ 1,48 h → passt zu den 90 min.

### 2. Solarpanel

Daten laut GalaxyRVR-Doku: ([SunFounder Dokumentation][2])

* Nennleistung: 6 V / 660 mA → ca. 4 W Peak
* Theoretische Voll-Ladezeit Akku: 7,2 h bei starkem Sonnenlicht
* Damit: 4 W · 7,2 h ≈ 29 Wh brutto, mit Wirkungsgradverlusten (Laderegler, Batterie) landet man grob wieder bei den ~15 Wh netto des Packs.

### 3. Was bringen deine Software-Optimierungen?

Du hast jetzt:

* RGB auf ca. 30 % Helligkeit (SoftPWM)
* Motor-Test auf ca. 45 % von `SpeedMax` im Cruise-Bereich
* Zeitlich begrenzte Tests (je 5 s pro Phase, Diagnoselauf insgesamt kurz)
* Sensor-Funktionen weitgehend nicht-blockierend

Grob gesagt:

* Wenn Motoren und LED zusammen z. B. 6–8 W bei Vollgas ziehen würden, bist du im Diagnosemodus eher im Bereich 2–4 W, und das nur für einige Sekunden.
* Gegenüber der Nenn-Laufzeit von 90 min unter „Spielbetrieb“ fällt dein Diagnoselauf energetisch kaum ins Gewicht – er ist deutlich „leichter“ als Dauervollgas mit Kamera/ESP32 etc.

Im Fahrbetrieb selbst gilt:

* Jeder Betrieb bei 30–60 % PWM statt 100 % senkt die mittlere Leistungsaufnahme der TT-Motoren deutlich.
* Dadurch streckst du die nutzbare Zeit pro Akku-Ladung; wie stark genau, hängt davon ab, wie viel Zeit der Rover wirklich fährt vs. steht, und wie viel die anderen Verbraucher (MCU, ESP32-CAM) ziehen.

### 4. Solar + Akku im Kontext

Mit dem 4-W-Solarpanel:

* kannst du bei gutem Licht den Energieverbrauch teilweise kompensieren,
* erreichst aber nur dann einen „Netto-Plus“, wenn der mittlere Verbrauch des Roboters unter ca. 4 W liegt.
* Deine Reduktionen bei PWM (Motoren, LED) bringen dich genau in diese Richtung, d. h. längere Betriebszeit mit gleicher Akkuladung und bessere Chance, dass das Panel im Stand oder im „Cruise“-Betrieb merkbar nachlädt.

[1]: https://docs.sunfounder.com/projects/galaxy-rvr/de/latest/hardware/cpn_18650_battery.html "18650 Batterie — SunFounder GalaxyRVR Kit for Arduino 1.0 Dokumentation"
[2]: https://docs.sunfounder.com/projects/galaxy-rvr/de/latest/hardware/cpn_solar_panel.html "Solarpanel — SunFounder GalaxyRVR Kit for Arduino 1.0 Dokumentation"
