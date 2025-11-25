# 1. ESP32-CAM (AI-Thinker) – Überblick

### 1.1 Grundprinzip

- ESP32-SoC (Dual-Core Tensilica, 240 MHz) + **OV2640-Kamera** auf einem Modul.
- Integriertes **WLAN (2,4 GHz)** und Bluetooth; kann Access-Point (AP), Station (STA) oder beides sein.
- Typische Firmware:
  - startet ein **WLAN-Netz** (AP oder verbindet sich mit deinem Router),
  - bietet einen **Webserver/Stream** (z. B. MJPEG) und/oder WebSocket,
  - kommuniziert über **UART** mit einem „Lower Computer“ (bei dir: SunFounder R3).
- Programmierung über **UART (U0TXD/U0RXD)** mit Boot-Pin **GPIO0** (Flash-Mode).

---

### 1.2 Wichtige Kenndaten

(grobe Richtwerte, Modul-Ebene)

| Kategorie               | Wert / Bereich                        |
|-------------------------|---------------------------------------|
| Versorgung              | 5 V am 5V-Pin (onboard Regler auf 3,3 V) |
| Logikspannung intern    | 3,3 V                                 |
| CPU                     | Dual-Core, bis 240 MHz                |
| RAM                     | 520 kB intern + 4 MB PSRAM (Modul)    |
| Flash                   | typ. 4 MB SPI-Flash                   |
| WLAN                    | 802.11 b/g/n, 2,4 GHz                 |
| Kamera                  | OV2640, max. 1600×1200 (UXGA), in Praxis oft QVGA–SVGA/VGA |
| Typischer Strom         | 80–200 mA bei aktivem WLAN und Kamera |

Für dein Galaxy-RVR-Projekt sind wichtig: stabile 5 V-Versorgung und ausreichend Strom (WLAN + Kamera ziehen deutlich mehr als der R3).

---

### 1.3 Pins (für dich relevant)

Viele Pins sind durch Kamera/SD-Karten-Interface belegt. Für Programmierung und Debugging sind vor allem diese wichtig:

| Modul-Pin   | ESP32-GPIO | Funktion                               |
|-------------|-----------:|----------------------------------------|
| 5V          | –          | Versorgungseingang 5 V                 |
| 3V3         | –          | interner 3,3-V-Rail (nicht als Eingang nutzen) |
| GND (mehrfach) | –       | Masse                                  |
| U0TXD       | GPIO1      | UART-TX (Daten von ESP32 zum PC/Arduino) |
| U0RXD       | GPIO3      | UART-RX (Daten vom PC/Arduino zum ESP32) |
| IO0         | GPIO0      | Boot-Pin: mit GND → Flash-Mode         |
| RST / EN    | EN         | Reset/Enable (Low → Reset)            |
| GPIO4       | D1/LED     | Kamera-/LED-Pin, nicht frei            |
| GPIO2, 12, 13, 14, 15 | – | von Kamera/SD belegt, nur mit Vorsicht nutzen |

Auf deiner **Kamera-Adapterplatine** sind U0TXD, U0RXD, 5 V, GND und RESET bereits auf den 7-poligen ZH-Stecker geführt; der R3 hängt darüber am UART.

---

### 1.4 Betriebsmodi (aus Programmier-Sicht)

1. **Boot-Modi (Flash vs. Run)**
   Gesteuert über `GPIO0` + `EN`:
   - `GPIO0` HIGH, EN Reset → **Normaler Betrieb** (Firmware läuft).
   - `GPIO0` LOW, EN Reset → **UART-Bootloader** (Flash-Modus, Programmieren mit esptool/PlatformIO).

2. **WLAN-Modi** (über die ESP-IDF/Arduino-API):
   - **STA**: verbindet sich mit bestehendem WLAN, bekommt IP vom Router.
   - **AP**: erzeugt eigenes WLAN (z. B. SSID `aiCAM`, IP 192.168.4.1).
   - **AP+STA**: beides gleichzeitig (seltener nötig im Rover-Setup).

3. **Energie-Modi**:
   - **Active**: CPU, WLAN und Kamera aktiv → hoher Strom, volle Funktion.
   - **Modem-Sleep / Light-Sleep**: CPU oder WLAN teilweise abgeschaltet, Timer/Interrupts wecken wieder auf.
   - **Deep-Sleep**: fast alles aus, Wake-Up über Timer, GPIO, ULP-Co-Prozessor; für Galaxy-RVR eher uninteressant.

4. **Kamera-Modi**:
   - konfiguriert über `framesize` (QVGA, VGA, SVGA, …), `pixformat` (JPEG, RGB), `fb_count` (Anzahl Frame-Buffer).
   - Trade-off: höhere Auflösung = mehr Bandbreite, mehr RAM, geringere FPS.

Für „später Programmieren“ merkst du dir:
Du startest typischerweise im **AP-Mode**, richtest einen **HTTP-Stream** und/oder **WebSocket-Server** ein, und nutzt den **UART** für Kommandos vom R3.

---

# 2. Kamera-Servo – Überblick

### 2.1 Grundprinzip

- Klassischer **RC-Servo** mit DC-Motor, Getriebe, Potentiometer und Elektronik.
- Du gibst nur ein **PWM-ähnliches Steuersignal** (Pulsbreite), der Servo regelt die Position selbst nach.
- Im Galaxy-RVR kippt der Servo die Kamera (Tilt).

---

### 2.2 Wichtige Kenndaten

(auf Basis der SunFounder-Angaben)

| Kategorie            | Wert / Bereich                   |
|----------------------|----------------------------------|
| Betriebsspannung     | 4,8 … 6,0 V DC                   |
| Leerlaufstrom        | ≈ 50–60 mA                       |
| Blockierstrom        | bis ≈ 550–650 mA (kurzzeitig)    |
| Nenndrehmoment       | ≥ 0,6–0,7 kgf·cm                 |
| Max. Drehmoment      | ≥ 1,4–1,6 kgf·cm                 |
| Stellzeit            | ≈ 0,14 s/60° @ 4,8 V             |
| Betriebswinkel       | ca. 180° (logisch)               |
| PWM-Signal           | ca. 50 Hz, Puls 500–2500 µs      |

---

### 2.3 Anschlüsse / Pins

Standard-RC-Servo, 3-adrig:

| Kabelfarbe | Funktion                | Anschluss am Shield                      |
|------------|-------------------------|------------------------------------------|
| Braun      | GND                     | GND des Servo-Headers                    |
| Rot        | VCC (5 V)               | 5 V des Servo-Headers                    |
| Orange     | PWM-Signal              | z. B. Arduino-Pin D6 („I6“ auf dem Shield) |

Auf dem Galaxy-RVR-Shield steckt der Kamera-Servo in der obersten Reihe des 2×4-Servo-Headers; das Signalkabel geht auf **D6**.

---

### 2.4 „Betriebsmodi“ aus Sicht des Codes

Servos haben keine komplexen Power-Modi, aber praktisch drei Zustände:

1. **Aktiv gesteuert**
   - Du erzeugst ein kontinuierliches PWM-Signal (z. B. mit `Servo`-Bibliothek oder eigener Timer-ISR).
   - Pulsbreite:
     - ca. 1 ms → eine Endstellung (z. B. 0°)
     - ca. 1,5 ms → Mitte (90°)
     - ca. 2 ms → andere Endstellung (180°)
   - Einige Servos akzeptieren 500–2500 µs für etwas mehr Weg.

2. **Haltekraft aus (PWM stoppen)**
   - Kein PWM → die Elektronik treibt den Motor nicht mehr.
   - Servo bleibt durch Reibung ungefähr in Position, kann aber durch Kräfte verdreht werden → geringerer Stromverbrauch.

3. **Stromlos**
   - 5 V weg (oder Transistor/Schalter dazwischen): komplett aus, keine Haltekraft.

Für späteres Programmieren merkst du dir:

- Im Code wird der Kamera-Servo auf dem Galaxy-RVR über **Pin D6** mit einer PWM-Pulsbreite gesteuert.
- Du brauchst nur „Winkel ↔ Pulsbreite“ zu mappen; die Regelung macht der Servo selbst.
- Stromspitzen beim Anfahren und Blockieren beachten (Versorgung über das Shield, nicht direkt vom R3-5V-Pin).

## Kamera-Adapterplatine – Pinbelegung

**HINWEIS**: QMC6310 – 3-Achsen-Magnetfeldsensor nicht verbaut im Mars-Rover

| Pin   | Bezeichnung | Funktion                                                                                 |
|-------|------------:|------------------------------------------------------------------------------------------|
| 1     | RST         | Reset-Leitung für die ESP32-CAM (setzt das Modul per Low-Puls zurück).                  |
| 2     | SCL         | I²C-SCL-Leitung für den QMC6310 (Kompass-Sensor auf dem Adapterboard).                  |
| 3     | SDA         | I²C-SDA-Leitung für den QMC6310.                                                         |
| 4     | RXD         | UART-RX der ESP32-CAM (U0RXD, GPIO3); empfängt Daten vom Arduino / USB-UART-Adapter.    |
| 5     | TXD         | UART-TX der ESP32-CAM (U0TXD, GPIO1); sendet Daten an Arduino / USB-UART-Adapter.       |
| 6     | 5V          | Versorgungseingang $5\,\mathrm{V}$ DC für ESP32-CAM und Adapterplatine.                 |
| 7     | GND         | Masseanschluss (0 V), Referenz für alle Signale.                                        |

---

## ESP32-CAM (AI-Thinker) – wichtige Pins

| Pin / GPIO  | Bezeichnung   | Funktion / Anschluss                                               |
|-------------|--------------:|--------------------------------------------------------------------|
| 5V          | 5V            | Versorgungseingang $4{,}75 \dots 5{,}25\,\mathrm{V}$ (mit Regler). |
| 3V3         | 3.3V          | interner $3{,}3\,\mathrm{V}$-Ausgang (nicht direkt einspeisen).   |
| GND (×3)    | GND           | Masse, mit System-GND zu verbinden.                               |
| GPIO1       | U0TXD / TX0   | UART-TX (serielle Daten von der ESP32-CAM) – an RX des Gegenübers.|
| GPIO3       | U0RXD / RX0   | UART-RX (serielle Daten zur ESP32-CAM) – an TX des Gegenübers.   |
| GPIO0       | IO0           | Boot-Pin: mit GND → Flash-Modus, offen → Normalbetrieb.          |
| GPIO14      | CLK           | SD-Karten-Interface: SPI-CLK.                                     |
| GPIO15      | CMD           | SD-Karten-Interface: Kommandoleitung.                            |
| GPIO2       | D0            | SD-Karten-Interface: Datenleitung 0.                             |
| GPIO4       | D1 / LED      | SD-Karten-Interface: Datenleitung 1; zugleich Board-LED.         |
| GPIO12      | D2            | SD-Karten-Interface: Datenleitung 2.                             |
| GPIO13      | D3            | SD-Karten-Interface: Datenleitung 3.                             |

---

## Kamera-Servo – technische Daten

| Parameter                       | Wert                                      | Kommentar                                      |
|---------------------------------|-------------------------------------------|------------------------------------------------|
| Motortyp                        | Kernmotor (DC-Servo)                      | klassischer RC-Servo                           |
| Betriebsspannung                | 4,8 … 6,0 V DC                            | Versorgung über 5 V-Schiene                    |
| Bereitschaftsstrom              | ≤ 4 mA                                   | ohne Bewegung                                  |
| Strom bei 4,8 V (Leerlauf)      | ≤ 50 mA                                  |                                                |
| Strom bei 6,0 V (Leerlauf)      | ≤ 60 mA                                  |                                                |
| Blockierstrom bei 4,8 V         | ≤ 550 mA                                 | kurzzeitig zulässig                            |
| Blockierstrom bei 6,0 V         | ≤ 650 mA                                 | kurzzeitig zulässig                            |
| Nenndrehmoment                  | 4,8 V: ≥ 0,6 kgf·cm; 6 V: ≥ 0,7 kgf·cm   | Dauerbetrieb                                   |
| Max. Drehmoment                 | 4,8 V: ≥ 1,4 kgf·cm; 6 V: ≥ 1,6 kgf·cm   | kurzzeitig                                     |
| Leerlaufdrehzahl                | 4,8 V: ≤ 0,14 s/60°; 6 V: ≤ 0,12 s/60°   | Stellzeit                                      |
| Betriebswinkel                  | ca. 180° ± 10° (500–2500 µs PWM)         | nutzbarer Stellbereich                         |
| Mechanischer Begrenzungswinkel  | 360°                                     | physische Anschläge                            |
| Betriebstemperaturbereich       | −10 °C … +50 °C                           |                                                |
| Lagerungstemperaturbereich      | −20 °C … +60 °C                           |                                                |
| Betriebsfeuchtigkeitsbereich    | ≤ 90 % r. F.                             |                                                |
| Lagerungsfeuchtigkeitsbereich   | ≤ 90 % r. F.                             |                                                |
| Gewicht                         | 10 ± 0,5 g                               | ohne Anbauteile                                |
| Material Gehäuse                | ABS                                      |                                                |
| Spiel / Mittelabweichung        | Spiel ≤ 4 µs; Mittellageabweichung ≤ 1° | Positioniergenauigkeit                         |
| Verstärkertyp                   | digital                                  | interne Elektronik                             |
| Kabellänge                      | 245 ± 5 mm (ohne Stecker)               | dreiadrig (GND, VCC, Signal)                   |
| Steckertyp                      | JR 2,54 mm / 3-Pin                       | Standard-Servoanschluss                        |

### Servo – Anschlussbelegung

| Kabelfarbe  | Funktion        |
|-------------|-----------------|
| Braun       | GND (Masse)     |
| Rot         | VCC (+4,8 … 6 V)|
| Orange      | PWM-Signaleingang |
