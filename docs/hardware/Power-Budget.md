# Power-Budget-Abschätzung

## 1. Eckdaten Akku, Solarpanel und Module

* Akku: 2× 18650, jeweils $2000\,\mathrm{mAh}$, nominell $7{,}4\,\mathrm{V}$
  ⇒ Energieinhalt $\approx 14{,}8\,\mathrm{Wh}$. ([SunFounder Dokumentation][1])
* Solarpanel: $6\,\mathrm{V}$, $660\,\mathrm{mA}$, theoretisch $\approx 7{,}2\,\mathrm{h}$ zum Vollladen des Akkus. ([SunFounder Dokumentation][2])
* TT-Motor (typisch, ähnlicher XH-TT-Motor): Nennspannung $3\,\mathrm{V} \dots 6\,\mathrm{V}$, No-Load-Strom $\approx 150\,\mathrm{mA}$ bei $6\,\mathrm{V}$.
  Laststrom deutlich höher, Stall $\gg 0{,}5\,\mathrm{A}$. ([SunFounder Dokumentation][3])
* RGB-Streifen (WS2812-Board im Kit): Versorgung $5\,\mathrm{V}$, Strom ca. $150\,\mathrm{mA}$ pro Segment. ([SunFounder Dokumentation][4])
* ESP32-CAM:
  - Flash aus: $I \approx 180\,\mathrm{mA}$ bei $5\,\mathrm{V}$
  - Flash max: $I \approx 310\,\mathrm{mA}$ bei $5\,\mathrm{V}$. ([SunFounder Dokumentation][5])

---

## 2. Typische Ströme pro Komponente (Annahmen bei 5 V)

```text
U = 5 V  (5-V-Schiene hinter dem Step-Down vom 2s-Akku)
````

| Komponente               | Annahme Strom (I)    | Leistung ($P \approx U\cdot I$)                      | Herleitung / Quelle                                               |
| ------------------------ | -------------------- | ---------------------------------------------------- | ----------------------------------------------------------------- |
| SunFounder R3 (UNO-Klon) | $50,\mathrm{mA}$     | $\approx 0{,}25,\mathrm{W}$                          | typische UNO-Leerlauflast (Schätzung)                             |
| ESP32-CAM (Flash aus)    | $180,\mathrm{mA}$    | $\approx 0{,}90,\mathrm{W}$                          | Datenblatt GalaxyRVR ([SunFounder Dokumentation][5])              |
| 2× TT-Motor „Cruise“     | je $300,\mathrm{mA}$ | $2 \times 1{,}5,\mathrm{W} \approx 3{,}0,\mathrm{W}$ | grobe Schätzung: ca. 2× No-Load bei $5,\mathrm{V}$                |
| 2× TT-Motor „Vollgas“    | je $600,\mathrm{mA}$ | $2 \times 3{,}0,\mathrm{W} \approx 6{,}0,\mathrm{W}$ | Schätzung im Bereich < Stall, aber hohe Last                      |
| RGB-Strip 100 %          | $150,\mathrm{mA}$    | $\approx 0{,}75,\mathrm{W}$                          | $5,\mathrm{V}$, $150,\mathrm{mA}$ ([SunFounder Dokumentation][4]) |
| RGB-Strip 30 % (PWM)     | $45,\mathrm{mA}$     | $\approx 0{,}23,\mathrm{W}$                          | ungefähr linear mit Duty-Cycle                                    |
| Servo (Kamera) aktiv     | $200,\mathrm{mA}$    | $\approx 1{,}0,\mathrm{W}$                           | typischer Micro-Servo unter Bewegung                              |
| Servo Ø im Fahrbetrieb   | $40,\mathrm{mA}$     | $\approx 0{,}20,\mathrm{W}$                          | wenig Bewegung, Mittelwert                                        |
| Sensoren (US + 2× IR)    | $20,\mathrm{mA}$     | $\approx 0{,}10,\mathrm{W}$                          | konservative Summe                                                |

Das ist bewusst „engineering-mäßig“: konservativ und rund, keine Milliwatt-Feilscherei.

---

## 3. Szenarien: Vollgas vs. „Cruise + gedimmt“

### 3.1 Summenströme

Annahmen:

* Versorgung aller 5-V-Verbraucher aus dem 2s-Akku über einen Wandler mit Wirkungsgrad $\eta \approx 0{,}85$ (nur für die Laufzeitabschätzung wichtig).
* Akku-Energie $E_\text{Akku} \approx 14{,}8,\mathrm{Wh}$.
* Spannungen im Bordnetz um $5,\mathrm{V}$.

#### Szenario A – „Idle + Streaming“

* R3: $50,\mathrm{mA}$
* ESP32-CAM: $180,\mathrm{mA}$
* Sensoren: $20,\mathrm{mA}$
* Motoren: aus
* RGB: aus
* Servo: nahezu aus (Leerlaufstrom kannst du bei Bedarf dazurechnen)

$I_\text{ges} \approx 0{,}05 + 0{,}18 + 0{,}02 = 0{,}25,\mathrm{A}$

$P_\text{ges} \approx 5,\mathrm{V} \cdot 0{,}25,\mathrm{A} = 1{,}25,\mathrm{W}$

#### Szenario B – „Cruise + gedimmt“

(so ähnlich wie dein Diagnose-Code: ~45 % SpeedMax, RGB auf 30 %)

* R3: $50,\mathrm{mA}$
* ESP32-CAM: $180,\mathrm{mA}$
* 2× TT-Motor „Cruise“: $600,\mathrm{mA}$
* RGB 30 %: $45,\mathrm{mA}$
* Servo im Mittel: $40,\mathrm{mA}$
* Sensoren: $20,\mathrm{mA}$

$I_\text{ges} \approx 0{,}05 + 0{,}18 + 0{,}60 + 0{,}045 + 0{,}04 + 0{,}02 \approx 0{,}935,\mathrm{A}$

$P_\text{ges} \approx 5,\mathrm{V} \cdot 0{,}935,\mathrm{A} \approx 4{,}68,\mathrm{W}$

#### Szenario C – „Vollgas + volle Helligkeit“

* R3: $50,\mathrm{mA}$
* ESP32-CAM (Flash an): $310,\mathrm{mA}$ ([SunFounder Dokumentation][5])
* 2× TT-Motor „Vollgas“: $1{,}2,\mathrm{A}$
* RGB 100 %: $150,\mathrm{mA}$
* Servo aktiver: $80,\mathrm{mA}$
* Sensoren: $20,\mathrm{mA}$

$I_\text{ges} \approx 0{,}05 + 0{,}31 + 1{,}20 + 0{,}15 + 0{,}08 + 0{,}02 \approx 1{,}81,\mathrm{A}$

$P_\text{ges} \approx 5,\mathrm{V} \cdot 1{,}81,\mathrm{A} \approx 9{,}05,\mathrm{W}$

---

### 3.2 Ideale Laufzeiten aus dem Akku

Ohne Verluste gilt näherungsweise:

$$
t \approx \frac{E_\text{Akku}}{P_\text{ges}}
$$

| Szenario                       | $P_\text{ges}$ [W] | Ideale Laufzeit $t$ [h] | in h:min (ideal) |
| ------------------------------ | ------------------ | ----------------------- | ---------------- |
| A: Idle + Streaming            | 1,25               | $\approx 11{,}8$        | ~11:50           |
| B: Cruise + 30 %-RGB           | 4,68               | $\approx 3{,}16$        | ~3:10            |
| C: Vollgas + volle RGB + Flash | 9,05               | $\approx 1{,}64$        | ~1:38            |

Realistisch musst du:

* DC/DC-Verluste (5-V-Regler, Motor-Treiber) und Innenwiderstand des Akkus berücksichtigen ($\eta$ eher $70 \dots 85,%$).
* Einen Teil der Kapazität ungenutzt lassen, weil die Abschaltspannung des Systems über der absoluten Entladeschwelle des Akkus liegt.

Wenn du z. B. einen Wirkungsgrad von $80,%$ annimmst, schrumpfen die Zeiten um denselben Faktor:

* Cruise-Szenario real eher ~2,5 h.
* Vollgas-Szenario eher ~1,3 h – passt grob zur offiziellen Angabe ~90–130 min Laufzeit für den Rover. ([CNX Software - Embedded Systems News][6])

---

## 4. Was bedeutet das für „Helligkeit“ und „Cruise-Speed“?

1. **RGB-Dimmung auf 30 %**

   * Volle RGB-Last: $\approx 0{,}75,\mathrm{W}$.
   * Bei 30 % PWM: $\approx 0{,}23,\mathrm{W}$.
     ⇒ Ersparnis $\approx 0{,}5,\mathrm{W}$.
     Im Cruise-Szenario ($\approx 4{,}7,\mathrm{W}$ gesamt) entspricht das grob **+10 % Laufzeit**.

2. **Cruise-Speed 30–60 % statt Dauer-Vollgas**

   * Motorleistung dominiert deutlich (ca. $3,\mathrm{W}$ Cruise vs. $6,\mathrm{W}$ Vollgas in der Annahme).
   * Allein der Wechsel von Vollgas → Cruise halbiert die Motorleistung und senkt das Gesamt-Power-Budget etwa von $\approx 9,\mathrm{W}$ auf $\approx 4{,}7,\mathrm{W}$.
     ⇒ Laufzeit springt von ca. $1{,}4,\mathrm{h}$ auf ca. $3,\mathrm{h}$ (ideale Rechnung) – in der Praxis eher **Richtung 2–2,5 h vs. knapp über 1 h**.

3. **Solarpanel-Einfluss**

   * Panel: $6,\mathrm{V} \cdot 0{,}66,\mathrm{A} \approx 4{,}0,\mathrm{W}$ maximal. ([SunFounder Dokumentation][2])
   * Nach Wandlungs- und Ladeverlusten kommen vielleicht $2{,}5 \dots 3,\mathrm{W}$ am Akku/5-V-Bus an.
   * Bei Cruise-Verbrauch $\approx 4{,}7,\mathrm{W}$ kann das Panel also **einen großen Teil** der Last tragen; die Batterie entlädt sich nur langsam.
   * Bei Vollgas-Verbrauch $\approx 9,\mathrm{W}$ ist das Panel nur ein Zu-Fütterer, verlängert die Laufzeit aber trotzdem merklich.
