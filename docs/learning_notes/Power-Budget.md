# Power-Budget-Abschätzung

## 1. Eckdaten Akku, Solarpanel und Module

* Akku: 2× 18650, jeweils (2000,\mathrm{mAh}), nominell (7{,}4,\mathrm{V})
  ⇒ Energieinhalt (\approx 14{,}8,\mathrm{Wh}). ([SunFounder Dokumentation][1])
* Solarpanel: (6,\mathrm{V}), (660,\mathrm{mA}), theoretisch ~(7{,}2,\mathrm{h}) zum Vollladen des Akkus. ([SunFounder Dokumentation][2])
* TT-Motor (typisch, ähnlicher XH-TT-Motor): Nennspannung (3\text{–}6,\mathrm{V}), No-Load-Strom ~(150,\mathrm{mA}) bei 6 V. Laststrom deutlich höher, Stall >> (0{,}5,\mathrm{A}). ([SunFounder Dokumentation][3])
* RGB-Streifen (WS2812-Board im Kit): Versorgung (5,\mathrm{V}), Strom ca. (150,\mathrm{mA}) pro Segment. ([SunFounder Dokumentation][4])
* ESP32-CAM:
  – Flash aus: (I \approx 180,\mathrm{mA} @ 5,\mathrm{V})
  – Flash max: (I \approx 310,\mathrm{mA} @ 5,\mathrm{V}). ([SunFounder Dokumentation][5])

---

## 2. Typische Ströme pro Komponente (Annahmen bei 5 V)

```text
U = 5 V (5-V-Schiene hinter dem Step-Down vom 2s-Akku)
```

| Komponente               | Annahme Strom (I) | Leistung (P \approx U\cdot I) | Herleitung / Quelle                                      |
| ------------------------ | ----------------- | ----------------------------- | -------------------------------------------------------- |
| SunFounder R3 (UNO-klon) | 50 mA             | 0,25 W                        | typische UNO-Leerlauflast (Schätzung)                    |
| ESP32-CAM (Flash aus)    | 180 mA            | 0,90 W                        | Datenblatt GalaxyRVR ([SunFounder Dokumentation][5])     |
| 2× TT-Motor, „Cruise“    | je 300 mA         | 2×1,5 W = 3,0 W               | grobe Schätzung: ca. 2× No-Load bei 5 V                  |
| 2× TT-Motor, „Vollgas“   | je 600 mA         | 2×3,0 W = 6,0 W               | Schätzung im Bereich < Stall, aber hohe Last             |
| RGB-Strip 100 %          | 150 mA            | 0,75 W                        | 5 V, 150 mA (Board-Spec) ([SunFounder Dokumentation][4]) |
| RGB-Strip 30 % (PWM)     | 45 mA             | 0,23 W                        | ~linear mit Duty-Cycle                                   |
| Servo (Kamera) aktiv     | 200 mA            | 1,0 W                         | typischer Micro-Servo unter Bewegung                     |
| Servo Ø im Fahrbetrieb   | 40 mA             | 0,20 W                        | wenig Bewegung, Mittelwert                               |
| Sensoren (US + 2× IR)    | 20 mA             | 0,10 W                        | konservative Summe                                       |

Das ist bewusst „engineering-mäßig“: konservativ und rund, keine Milliwatt-Feilscherei.

---

## 3. Szenarien: Vollgas vs. „Cruise + gedimmt“

### 3.1 Summenströme

Annahmen:

* Versorgung aller 5-V-Verbraucher aus dem 2s-Akku über einen Wandler mit Wirkungsgrad (\eta \approx 85,%) (nur für die Laufzeitabschätzung wichtig).
* Akku-Energie (E_\text{Akku} \approx 14{,}8,\mathrm{Wh}).
* Spannungen im Bordnetz um (5,\mathrm{V}).

#### Szenario A – „Idle + Streaming“

* R3: 50 mA
* ESP32-CAM: 180 mA
* Sensoren: 20 mA
* Motoren: aus
* RGB: aus
* Servo: nahezu aus (20 mA Leerlauf kannst du bei Bedarf dazurechnen)

[
I_\text{ges} \approx 0{,}05 + 0{,}18 + 0{,}02 = 0{,}25,\mathrm{A}
]

[
P_\text{ges} \approx 5,\mathrm{V}\cdot 0{,}25,\mathrm{A} = 1{,}25,\mathrm{W}
]

#### Szenario B – „Cruise + gedimmt“

(so ähnlich wie dein Diagnose-Code: ~45 % SpeedMax, RGB auf 30 %)

* R3: 50 mA
* ESP32-CAM: 180 mA
* 2× TT-Motor „Cruise“: 600 mA
* RGB 30 %: 45 mA
* Servo im Mittel: 40 mA
* Sensoren: 20 mA

[
I_\text{ges} \approx 0{,}05 + 0{,}18 + 0{,}60 + 0{,}045 + 0{,}04 + 0{,}02
\approx 0{,}935,\mathrm{A}
]

[
P_\text{ges} \approx 5,\mathrm{V}\cdot 0{,}935,\mathrm{A} \approx 4{,}68,\mathrm{W}
]

#### Szenario C – „Vollgas + volle Helligkeit“

* R3: 50 mA
* ESP32-CAM (Flash an): 310 mA ([SunFounder Dokumentation][5])
* 2× TT-Motor „Vollgas“: 1,2 A
* RGB 100 %: 150 mA
* Servo aktiv(er): 80 mA
* Sensoren: 20 mA

[
I_\text{ges} \approx 0{,}05 + 0{,}31 + 1{,}20 + 0{,}15 + 0{,}08 + 0{,}02
\approx 1{,}81,\mathrm{A}
]

[
P_\text{ges} \approx 5,\mathrm{V}\cdot 1{,}81,\mathrm{A} \approx 9{,}05,\mathrm{W}
]

---

### 3.2 Ideale Laufzeiten aus dem Akku

Ohne Verluste:

[
t \approx \frac{E_\text{Akku}}{P_\text{ges}}
]

| Szenario                       | (P_\text{ges}) [W] | Ideale Laufzeit (t) [h] | in h:min (ideal) |
| ------------------------------ | ------------------ | ----------------------- | ---------------- |
| A: Idle + Streaming            | 1,25               | 11,8                    | ~11:50           |
| B: Cruise + 30 %-RGB           | 4,68               | 3,17                    | ~3:10            |
| C: Vollgas + volle RGB + Flash | 9,05               | 1,64                    | ~1:38            |

Realistisch musst du:

* DC/DC-Verluste (5-V-Regler, Motor-Treiber) und Innenwiderstand des Akkus berücksichtigen ((\eta) eher 70–85 %).
* Ein Teil der Kapazität bleibt wegen Spannungsabschaltung ungenutzt.

If du z. B. 80 % Wirkungsgrad annimmst, schrumpfen die Zeiten um denselben Faktor:

* Cruise-Szenario realistisch eher ~2,5 h.
* Vollgas-Szenario eher ~1,3 h – passt grob zur offiziellen Angabe ~90–130 min Laufzeit für den Rover. ([CNX Software - Embedded Systems News][6])

---

## 4. Was bedeutet das für „Helligkeit“ und „Cruise-Speed“?

1. **RGB-Dimmung auf 30 %**

   * Volle RGB-Last: ~0,75 W.
   * Bei 30 % PWM: ~0,23 W.
     ⇒ Ersparnis ~0,5 W.
     Im Cruise-Szenario (~4,7 W gesamt) entspricht das grob **+10 % Laufzeit**.

2. **Cruise-Speed 30–60 % statt Dauer-Vollgas**

   * Motorleistung dominiert deutlich (3 W Cruise vs. 6 W Vollgas in der Annahme).
   * Allein der Wechsel von Vollgas → Cruise halbiert die Motorleistung und senkt das Gesamt-Power-Budget etwa von ~9 W auf ~4,7 W.
     ⇒ Laufzeit springt von ca. 1,4 h auf ca. 3 h (ideale Rechnung) – in der Praxis eher **Richtung 2–2,5 h vs. knapp über 1 h**.

3. **Solarpanel-Einfluss**

   * Panel: (6,\mathrm{V}\cdot 0{,}66,\mathrm{A} \approx 4{,}0,\mathrm{W}) maximal. ([SunFounder Dokumentation][2])
   * Nach Wandlungs- und Ladeverlusten kommen vielleicht 2,5–3 W am Akku/5-V-Bus an.
   * Bei Cruise-Verbrauch ~4,7 W kann das Panel also **einen großen Teil** der Last tragen; die Batterie entlädt sich nur langsam.
   * Bei Vollgas-Verbrauch ~9 W ist das Panel nur ein Zu-Fütterer, aber kann die Laufzeit trotzdem merklich verlängern.

Kurzfassung:

* Ja, **Helligkeits-Dimmung** und **Cruise-Speed im Bereich 30–60 % von SpeedMax** sparen spürbar Akku.
* Im Praxis-Mix „viel Cruise, gelegentlich Vollgas, RGB oft gedimmt“ landest du sehr wahrscheinlich in einem Bereich, der zur offiziellen Akku-Laufzeit (90–130 min) passt – das Panel obendrauf verschiebt das weiter nach oben, wenn genug Licht da ist.

[1]: https://docs.sunfounder.com/projects/galaxy-rvr/en/latest/hardware/cpn_18650_battery.html?utm_source=chatgpt.com "18650 Battery — SunFounder GalaxyRVR Kit for Arduino ..."
[2]: https://docs.sunfounder.com/projects/galaxy-rvr/de/latest/hardware/cpn_solar_panel.html?utm_source=chatgpt.com "Solarpanel — SunFounder GalaxyRVR Kit for Arduino 1.0 ..."
[3]: https://docs.sunfounder.com/projects/zeus-car/en/latest/hardware/cpn_tt_motor_xh.html?utm_source=chatgpt.com "TT Motor — SunFounder Zeus Robot Car Kit ..."
[4]: https://docs.sunfounder.com/projects/galaxy-rvr/en/latest/hardware/cpn_rgb_strip.html?utm_source=chatgpt.com "4 RGB LEDs Strip"
[5]: https://docs.sunfounder.com/projects/galaxy-rvr/de/latest/hardware/cpn_esp_32_cam.html?utm_source=chatgpt.com "ESP32 CAM — SunFounder GalaxyRVR Kit for Arduino 1.0 ..."
[6]: https://www.cnx-software.com/2024/01/29/sunfounder-galaxyrvr-review-arduino-programmable-mars-rover-robot/?utm_source=chatgpt.com "SunFounder GalaxyRVR review - An Arduino ..."
