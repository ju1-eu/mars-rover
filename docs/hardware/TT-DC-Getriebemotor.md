# TT-DC-Getriebemotor – Überblick

### 1. Grundprinzip

- Im Kern steckt ein **einfacher, bürstenbehafteter DC-Motor** (Permanentmagnet-Anker).
- Dahinter sitzt ein **Stirnrad-Getriebe** mit großem Übersetzungsverhältnis
  \[
  i = 1 : 120
  \]
  → hohe Übersetzung, dadurch:
  - **Drehzahl stark reduziert**,
  - **Drehmoment stark erhöht**,
  - Motor eignet sich für langsame, kraftvolle Fahrantriebe.
- Der Motor hat **nur zwei Anschlüsse**:
  - Spannung in eine Richtung → Motor dreht vorwärts.
  - Polarität umkehren → Motor dreht rückwärts.
- Auf dem GalaxyRVR werden **je drei Motoren pro Seite parallel** an einen H-Brückentreiber gehängt (Left/Right).

---

### 2. Wichtige Kenndaten (für Berechnungen)

| Parameter                | Wert                          | Hinweis                             |
|--------------------------|------------------------------|-------------------------------------|
| Empfohlene Betriebsspannung | 3,0 … 4,5 V DC             | Rover läuft typ. mit ~3–4,5 V       |
| Übersetzung              | 1 : 120                       | Motor hochdrehend, Abtrieb langsam  |
| Leerlaufstrom            | ca. 130 mA                    | pro Motor                           |
| Leerlaufdrehzahl         | ca. 38 rpm ± 8 %              | an der Abtriebswelle                |
| Startspannung (Leerlauf) | max. 2 V                      | darunter läuft er evtl. nicht an    |
| Ausgangsdrehmoment       | bei 3 V ≥ 1,2 kgf·cm          | Dauer/Nennmoment                    |
| Drehrichtung             | beide Richtungen              | per Polaritätswechsel / H-Brücke    |
| Lebensdauer (nutzbar)    | 70 … 120 h                    | Herstellerangabe                    |
| Abmessungen              | 70 × 22,5 × 36,6 mm           | L × B × H                           |
| Stecker                  | XH2.54-2P, 250 mm Kabel       | grau/schwarz                        |

Für grobe Bewegungsrechnungen kannst du später aus Drehzahl + Radradius die Fahrgeschwindigkeit bestimmen:
\[
v = \frac{2\pi r \cdot n}{60}
\]
mit \(r\) = Radradius, \(n\) = Drehzahl in rpm.

---

### 3. Pins & Anschlüsse

#### Motor selbst

| Anschluss | Funktion          |
|----------:|-------------------|
| 1         | Motorleitung A    |
| 2         | Motorleitung B    |

- Vertauschst du A/B am H-Brückenausgang, kehrt sich **„vorwärts“/„rückwärts“** softwareseitig um.

#### Anbindung im GalaxyRVR

- **Motor Port Left:** 3 Buchsen, 3 Motoren links → alle an einem Treiber, gesteuert über Arduino-Pins **D2** und **D3**.
- **Motor Port Right:** 3 Buchsen, 3 Motoren rechts → anderer Treiber, gesteuert über **D4** und **D5**.

Der Treiber auf dem Shield ist eine **H-Brücke**; du steuerst ihn nur über die Logikeingänge (Arduino-Pins), nicht direkt die Motorleitungen.

---

### 4. Betriebsmodi (aus Sicht der Ansteuerlogik)

Für jede Seite (Left / Right) gibt es zwei Eingangssignale der H-Brücke:

- Left:  `IN1_L = D2`, `IN2_L = D3`
- Right: `IN1_R = D4`, `IN2_R = D5`

Typische Wahrheits-Tabelle (pro Seite):

| IN1 | IN2 | Motorzustand      | Bemerkung                         |
|----:|----:|-------------------|-----------------------------------|
| 0   | 0   | Freilauf (Coast)  | Motor läuft aus, kaum Bremsmoment|
| 1   | 0   | Drehen vorwärts   | Richtung je nach Verdrahtung     |
| 0   | 1   | Drehen rückwärts  | umgekehrte Drehrichtung          |
| 1   | 1   | Elektr. Bremse    | Kurzschlussbremse, Motor stoppt schneller |

**Geschwindigkeitsregelung** machst du später per **PWM**:

- Einen der beiden Eingänge (z. B. IN1) mit `analogWrite()` pwm-modulieren,
- den anderen Eingang konstant HIGH/LOW setzen (Richtung).

Beispiel (Pseudocode, eine Seite):

```cpp
// Vorwärts, 50 % Geschwindigkeit
digitalWrite(IN2_L, LOW);        // Richtung
analogWrite(IN1_L, 128);         // 0..255 => 0..100 % PWM

// Rückwärts, 100 % Geschwindigkeit
digitalWrite(IN1_L, LOW);
analogWrite(IN2_L, 255);
```


## TT-DC-Getriebemotor – technische Daten

| Parameter                 | Wert                              | Kommentar                                       |
|---------------------------|-----------------------------------|-------------------------------------------------|
| Empfohlene Spannung       | 3,0 … 4,5 V DC                    | Betrieb im Rover typ. mit ca. 3–4,5 V          |
| Übersetzungsverhältnis    | 1 : 120                           | hohes Drehmoment, geringe Drehzahl             |
| Wellenanzahl              | Einzelwelle                       | einfache Abtriebswelle                         |
| Leerlaufstrom             | ca. 130 mA                        | bei Nennspannung                               |
| Leerlaufgeschwindigkeit   | ca. 38 rpm ± 8 %                  | an Abtriebswelle                               |
| Startspannung             | max. 2 V (Leerlauf)               | darunter läuft der Motor nicht sicher an       |
| Ausgangsdrehmoment        | bei 3 V ≥ 1,2 kgf·cm              | Dauer/Nennmoment                               |
| Nutzbare Lebensdauer      | 70 … 120 h                        | Herstellerangabe                               |
| Drehrichtung              | beide Richtungen                  | über Polaritätswechsel                          |
| Abmessungen (Körper)      | 70 × 22,5 × 36,6 mm               | L × B × H                                      |
| Kabel                     | grau & schwarz, 24 AWG, 250 mm    | zweiadrig                                      |
| Stecker                   | XH2.54-2P, weiß                   | passt zu Motor-Port des Galaxy-RVR-Shields     |
| Gewicht                   | 28,5 g                            |                                               |
