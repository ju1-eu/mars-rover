# Überblick QMC6310 – 3-Achsen-Magnetfeldsensor

**HINWEIS**: QMC6310 – 3-Achsen-Magnetfeldsensor nicht verbaut im Mars-Rover

Kurze Einordnung: Der QMC6310 ist ein dreiachsiger Magnetfeldsensor (Magnetometer) auf AMR-Basis, der auf der Kamera-Adapterplatine sitzt. Er liefert bereits digital kalibrierte Messwerte über I²C und ist für E-Kompass-Anwendungen gedacht.

---

### 1. Grundprinzip

- **Sensorprinzip:** Anisotropic Magneto-Resistive (AMR). Drei Brückensensoren messen das Magnetfeld in X-, Y- und Z-Richtung.
- **Signalpfad im Chip:** AMR-Brücke → Multiplexer → programmierbarer Verstärker (PGA) → 16-Bit-ADC → digitale Signalaufbereitung (Offset-Korrektur, Temperaturkompensation) → Register.
- **Zweck:** Typisch \(1 \dots 2^\circ\) Kompassgenauigkeit für Heading/Navigation.

---

### 2. Wichtige Kenndaten (für späteres Programmieren)

| Parameter                | Wert / Bereich                                                  |
| ------------------------ | --------------------------------------------------------------- |
| Versorgung \((V_\text{DD})\) | \(2{,}5\,\mathrm{V} \dots 3{,}6\,\mathrm{V}\)                   |
| Messbereich (Full Scale) | \(\pm 2 / \pm 8 / \pm 12 / \pm 30\,\mathrm{G}\)                 |
| Empfindlichkeit          | z. B. Bereich \(\pm 2\,\mathrm{G}\): \(15000\,\mathrm{LSB/G}\)  |
| Auflösung                | bis ca. \(1\,\mathrm{mG}\) (≈ \(0{,}1\,\mu\mathrm{T}\))         |
| Ausgangsformat           | 3× 16-Bit Zweierkomplement (X, Y, Z)                            |
| Schnittstelle            | I²C, 7-Bit-Adresse `0x1C` oder `0x3C`                           |
| Max. Datenrate           | bis \(1{,}5\,\mathrm{kHz}\)                                     |
| Ruhestrom „Normal Mode“  | ca. \(35 \dots 600\,\mu\mathrm{A}\) (je nach ODR/OSR)           |
| Temperaturbereich        | \(-40^\circ\mathrm{C} \dots 85^\circ\mathrm{C}\)                |

Für Kompass-Betrieb im Rover wirst du praktisch immer den **Bereich \(\pm 2\,\mathrm{G}\)** wählen: höchste Auflösung, Erdmagnetfeld liegt bei ~\(0{,}5\,\mathrm{G}\).

Umrechnung Rohwert → Feldstärke z. B. bei \(\pm 2\,\mathrm{G}\):

- Sensitivität \(S = 15000\,\mathrm{LSB/G}\)
- Rohwert \(N\) (signed 16-Bit):

\[
B[\mathrm{G}] = \frac{N}{S}, \qquad
B[\mu\mathrm{T}] = \frac{N}{S} \cdot 100
\]

---

### 3. Pins & Anschluss (auf der Adapterplatine schon erledigt)

Der nackte QMC6310 hat nur wenige Anschlüsse:

- **VDD** – Versorgung (3{,}3 V)
- **VSS** – Masse
- **SCK** – I²C-Clock
- **SDA** – I²C-Daten
- **CAP** – Anschluss für externen \(4{,}7\,\mu\mathrm{F}\)-Kondensator (Stützkondensator)

Auf deiner SunFounder Kamera-Adapterplatine ist das alles schon fertig: der QMC6310 hängt an der 5-V-Versorgung der Platine (intern geregelt) und an den I²C-Leitungen SCL/SDA, die zum R3-Board geführt werden. Du musst beim Programmieren nur den **I²C-Bus** benutzen.

---

### 4. Betriebsmodi (Power-Management)

Die Betriebsart wird über **Control-Register 1 (`0x0A`)** gesteuert:

- **MODE\<1:0\>**
  - `00` – Suspend (Default nach Power-On-Reset, keine Messung, I²C aktiv)
  - `01` – Normal Mode (periodische Messungen)
  - `10` – Single Mode (eine Messung → zurück in Suspend)
  - `11` – Continuous Mode (dauernde Messungen, max. ODR)

- **ODR\<1:0\>** (Output Data Rate) – 10 Hz, 50 Hz, 100 Hz, 200 Hz
- **OSR1\<1:0\>, OSR2\<1:0\>** – Oversampling / Digitalfilter: höherer Wert = weniger Rauschen, mehr Strom.

Typischer Startpunkt für den Rover-Kompass:

- **Mode:** Continuous
- **ODR:** 50–100 Hz
- **OSR:** mittlere Einstellungen (z. B. OSR1=4, OSR2=2) als Kompromiss aus Rauschen und Strom.

Die Reichweite und Set/Reset-Modi stellst du mit **Control-Register 2 (`0x0B`)** ein (Bits RNG\<1:0\> und SET/RESET MODE\<1:0\>).

---

### 5. Register, die du fürs Programmieren wirklich brauchst

Mindest-Satz an Registern:

| Adresse | Name                   | Zweck im Code                          |
| ------: | ---------------------- | -------------------------------------- |
| `0x00`  | CHIPID                 | sollte `0x80` liefern → Sensor erkannt |
| `0x01`/`0x02` | XOUT LSB/MSB     | X-Achse (16-Bit, Zweierkomplement)     |
| `0x03`/`0x04` | YOUT LSB/MSB     | Y-Achse                                |
| `0x05`/`0x06` | ZOUT LSB/MSB     | Z-Achse                                |
| `0x09`  | STATUS (DRDY/OVFL)     | Daten fertig? Überlauf?                |
| `0x0A`  | CTRL1 (MODE, ODR, OSR) | Betriebsmodus, Datenrate, Oversampling |
| `0x0B`  | CTRL2 (RNG, Reset)     | Messbereich, Self-Test, Soft-Reset     |

Typischer Messzyklus im Programm:

1. **Initialisierung**
   - I²C starten (100 kHz oder 400 kHz).
   - Register `0x29` (Axis-Sign) mit `0x06` beschreiben (Achsen-Vorzeichen wie im Datenblatt).
   - Register `0x0B`: gewünschten **RNG** (z. B. \(\pm 2\,\mathrm{G}\)) und Set/Reset-Modus setzen.
   - Register `0x0A`: MODE = Continuous oder Normal, passende ODR/OSR.

2. **Loop**
   - Status-Register `0x09` lesen; wenn DRDY=1 → Daten bereit.
   - Register `0x01`–`0x06` nacheinander lesen (X, Y, Z).
   - 16-Bit-Werte zusammensetzen, mit Sensitivität skalieren → \(\mu\mathrm{T}\).

3. **Optional**
   - Self-Test: Bit `SELF_TEST` in `0x0B` setzen, Werte vorher/nachher vergleichen.
   - Soft-Reset: Bit `SOFT_RST` in `0x0B` setzen, um Register auf Default zurückzusetzen.

---

### 6. Was du dir für später merken solltest

Für „später Programmieren“ reicht es, wenn du jetzt die folgenden Punkte im Kopf hast:

- QMC6310 misst **dreidimensionales Magnetfeld** (X/Y/Z) und hängt über **I²C** am R3.
- Du arbeitest hauptsächlich mit **Registern `0x01`–`0x06`, `0x09`, `0x0A`, `0x0B`**.
- Für Kompass: **Range \(\pm 2\,\mathrm{G}\)**, Continuous Mode, ODR 50–100 Hz, mittleres Oversampling.
- Rohwerte sind 16-Bit signed; durch Division mit der Sensitivität (z. B. \(15000\,\mathrm{LSB/G}\)) bekommst du Feldstärken in Gauss bzw. Mikrotesla.

---

## Kamera-Adapterplatine mit QMC6310 – Übersicht

### Adapterplatine

| Parameter            | Wert                     | Kommentar                                                 |
| -------------------- | ------------------------ | --------------------------------------------------------- |
| Funktion             | Kamera-Adapterboard      | Träger für ESP32-CAM und QMC6310-Kompasssensor.          |
| Betriebsspannung     | 5 V                      | Versorgung über Galaxy-RVR-Shield oder externes 5-V-Rail. |
| Stecker zum Shield   | ZH1.5, 7-polig           | Führt UART, I²C, 5 V und GND zum Hauptsystem.            |
| Abmessungen          | 40 mm × 27 mm × 15 mm    | Platinenmaß inkl. bestückter Bauteile.                   |
| Protokolle auf Board | UART und I²C             | UART für ESP32-CAM, I²C für QMC6310.                     |

### QMC6310 Magnetfeldsensor auf der Platine

| Parameter              | Wert                         | Kommentar                                                   |
| ---------------------- | ---------------------------- | ----------------------------------------------------------- |
| Sensortyp              | QMC6310                      | 3-Achs-Magnetfeldsensor (AMR) im LGA-8-Gehäuse.             |
| Versorgung (intern)    | \(2{,}5\,\mathrm{V} \dots 3{,}6\,\mathrm{V}\) | Auf der Adapterplatine aus 5 V geregelt.                |
| Auflösung / ADC        | 16-Bit                       | Feldauflösung bis ca. \(2\,\mathrm{mG}\) bei geeigneter Konfiguration. |
| Messbereich            | \(\pm 30\,\mathrm{G}\)       | Weitere Bereiche \(\pm 12\,\mathrm{G}\), \(\pm 8\,\mathrm{G}\), \(\pm 2\,\mathrm{G}\) per Register wählbar. |
| Schnittstelle          | I²C                          | Standard/Fast-Mode (100 kHz / 400 kHz).                     |
| I²C-Adresse (7-Bit)    | `0x1C` oder `0x3C`           | Je nach Variante QMC6310U bzw. QMC6310N.                    |
| Typischer Strom        | ≈ \(35\,\mu\mathrm{A}\) @ 10 Hz (Low-Power) | Für batterie- und solarbetriebene Anwendungen geeignet. |
