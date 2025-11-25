# RGB-LED-Streifen – Überblick

### 1. Grundprinzip

- Auf jedem Streifen sitzen mehrere **5050-RGB-LEDs**.
- Jede 5050-LED enthält **3 einzelne LEDs**:
  - Rot (R), Grün (G), Blau (B).
- Im Galaxy-RVR-Kit sind sie als **analoge RGB-Streifen mit gemeinsamer Anode** aufgebaut:
  - **+5V** ist gemeinsam für alle drei Farben.
  - Über die drei Leitungen **R, G, B** werden die **Kathoden** geschaltet.
- Helligkeit und Farbe entstehen durch **Mischen der drei Farbkanäle**:
  - jeder Kanal wird mit **PWM** gedimmt,
  - daraus ergeben sich die sichtbaren Farben (Additive Farbmischung).

Es sind **4 identische RGB-Streifen**, die am Shield parallel hängen und immer dieselbe Farbe zeigen.

---

### 2. Kenndaten

| Parameter              | Wert / Bereich          | Hinweis                              |
|------------------------|------------------------|--------------------------------------|
| Arbeitsspannung        | 5 V DC                 | direkte Versorgung vom Shield        |
| LED-Typ               | 5050 RGB               | je Chip: R, G, B                     |
| Farbe                  | Vollfarbe RGB          | Farbmischung per PWM                 |
| Nennstrom (ein Streifen) | ca. 150 mA          | bei Vollweiß (alle Kanäle an)       |
| Leistung (ein Streifen)| ca. 1,5 W              | 5 V × 0,3 A (alle Kanäle beidseitig) |
| Temperaturbereich      | −15 °C … +50 °C        | laut Datenblatt                      |
| Streifenbreite         | 5,5 mm                 |                                      |
| Streifendicke          | 2 mm                   |
| Kabel                  | ZH1.5-4P, 25 cm, 28 AWG| Steckbar ins Shield                  |

> Für Dimensionierung: 4 Streifen × 150 mA ≈ 600 mA bei „Vollweiß“.

---

### 3. Pins am RGB-Streifen / Shield

Der 4-polige Stecker (ZH1.5-4P) hat folgende Belegung:

| Pin | Signal | Funktion                              | Arduino-Pin am Shield |
|-----|--------|----------------------------------------|------------------------|
| 1   | +5V    | gemeinsame Anode aller LEDs           | –                      |
| 2   | B      | Kathode Blau                          | D11                    |
| 3   | R      | Kathode Rot                           | D12                    |
| 4   | G      | Kathode Grün                          | D13                    |

Beide RGB-Buchsen auf dem Shield sind **parallel** geschaltet: B/R/G für alle Streifen hängen an denselben Arduino-Pins.

Im Code steuerst du also **genau drei Pins (D11, D12, D13)** und damit alle 4 Streifen gleichzeitig.

---

### 4. Betriebsmodi (aus Sicht des Programms)

Elektrisch gibt es nur „an/aus“ pro Farbe, aber durch PWM ergeben sich verschiedene „Modi“:

1. **Feste Farbe**
   - Drei PWM-Werte `0…255` für R, G, B wählen.
   - Beispiel:
     - Rot = (255, 0, 0) → nur R-Versorgung aktiv.
     - Grün = (0, 255, 0), Blau = (0, 0, 255).
     - Weiß ≈ (255, 255, 255).

2. **Helligkeitssteuerung**
   - Skalierung aller drei Kanäle mit einem Faktor:
     \[
     R' = k \cdot R,\quad G' = k \cdot G,\quad B' = k \cdot B
     \]
   - `k` zwischen 0 (aus) und 1 (volle Helligkeit).

3. **Blinken / Statusanzeige**
   - Farbe ein/aus schalten oder zwischen zwei Farben umschalten.
   - Anwendung: Fehlermeldungen, Betriebszustand (z. B. „Verbunden“, „Laden“, „Fehler“).

4. **Dynamische Effekte**
   - **Farbverläufe / Fades**: R, G, B über Zeit weich verändern.
   - **Pulsieren / Atmen**: Helligkeit periodisch hoch- und runterfahren.
   - **Richtungsanzeige**: Farbe abhängig von Fahrmodus (z. B. vorwärts = grün, rückwärts = rot).

5. **Energiesparen**
   - Alle drei Kanäle auf `0` setzen → Stromaufnahme ≈ 0.
   - Helligkeit reduzieren (z. B. max. 50 % PWM), um Strom und Akku zu schonen.

Merken für späteres Programmieren:

- Du steuerst **immer drei PWM-Kanäle (D11, D12, D13)**.
- Logische „Modi“ (Farbe, Blinken, Effekte) sind reine Software; die Hardware bleibt ein analoger RGB-Streifen mit gemeinsamer +5 V-Anode.

---

## RGB-LED-Streifen – technische Daten

| Parameter                | Wert                        | Kommentar                                        |
|--------------------------|-----------------------------|--------------------------------------------------|
| Arbeitsspannung          | 5 V DC                      | Versorgung über Galaxy-RVR-Shield                |
| Farbe                    | Vollfarbe RGB              | Additive Farbmischung über R/G/B-Kanäle         |
| LED-Typ                  | 5050 RGB                    | Standard-Multi-LED-Gehäuse                       |
| Nennstrom                | 150 mA (einzelschaltkreis) | pro Streifen bei Vollweiß                        |
| Leistung                 | 1,5 W                       | Herstellerangabe je Streifen                     |
| Betriebstemperatur       | −15 °C … +50 °C             | gemäß Datenblatt                                 |
| Streifendicke            | 2 mm                        | mechanische Bauhöhe                              |
| Streifenbreite           | 5,5 mm                      | passend für Unterboden-Montage                   |
| Kabel / Anschluss        | ZH1.5-4P, 25 cm, 28 AWG     | steckfertig für RGB-Port des Galaxy-RVR-Shields  |

### RGB-LED-Streifen – Pinbelegung ZH1.5-4P

| Pin | Bezeichnung | Funktion                                                      |
|-----|------------:|---------------------------------------------------------------|
| 1   | +5V         | gemeinsame Anode aller drei LEDs, an +5 V DC anzuschließen   |
| 2   | B           | Kathode der blauen LED (Blau-Kanal)                          |
| 3   | R           | Kathode der roten LED (Rot-Kanal)                            |
| 4   | G           | Kathode der grünen LED (Grün-Kanal)                          |


