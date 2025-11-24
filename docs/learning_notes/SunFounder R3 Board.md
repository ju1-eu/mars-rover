# Überblick: SunFounder R3 Board (Arduino-Uno-kompatibel)

### 1. Grundprinzip

* Herzstück ist ein **ATmega328P**: 8-bit AVR-Mikrocontroller mit RISC-Architektur, Harvard-Design (Programm- und Datenspeicher getrennt), 32 Register, bis 16 MHz Takt.
* Programmspeicher: **32 kB Flash**, davon ca. 0,5 kB für den Bootloader; **2 kB SRAM**, **1 kB EEPROM** für dauerhafte Daten.
* Auf dem Board sitzt zusätzlich ein **USB-zu-Serial-Controller (ATmega16U2)**, der die USB-Buchse mit dem UART des ATmega328P verbindet → Programmierung und serielles Debugging über USB.
* Spannungsregler und Schutzbeschaltung erzeugen **5 V-Logikspannung** aus USB oder 7–12 V am DC-Jack; ein 3,3-V-Regler versorgt den 3,3-V-Pin.
* Der Mikrocontroller startet nach Reset im **Bootloader**, lauscht kurz auf neue Firmware und springt dann in dein Sketch.

Für dich heißt das: Du programmierst immer den ATmega328P; das Board kümmert sich um USB, Spannungsversorgung und Anschlusspins.

---

### 2. Wichtige Kenndaten (Hardware-Level)

| Kategorie          | Wert / Bereich                        | Hinweis für die Praxis                      |
| ------------------ | ------------------------------------- | ------------------------------------------- |
| Mikrocontroller    | ATmega328P                            | AVR-8-bit, 16 MHz max.                      |
| Logikspannung      | 5 V                                   | Alle I/Os 5-V-Logik.                        |
| Versorgung Board   | 7–12 V (DC-Jack), 5 V (USB)           | 6–20 V absolut möglich, thermisch begrenzt. |
| Digitale I/O-Pins  | 14 (D0–D13)                           | Davon 6 PWM: D3, D5, D6, D9, D10, D11.      |
| Analoge Eingänge   | 6 (A0–A5), 10-bit ADC                 | Referenz über AREF oder Vcc.                |
| Max. Strom pro I/O | 20 mA (empfohlen)                     | Summe über alle Pins begrenzen.             |
| 3,3-V-Pin          | max. ca. 50 mA                        | Für kleine Sensoren.                        |
| Flash/SRAM/EEPROM  | 32 kB / 2 kB / 1 kB                   | Für Sketch, Variablen, permanente Daten.    |
| Takt               | 16 MHz Quarz                          | Standard-Arduino-Takt.                      |
| Onboard-LED        | D13 (`LED_BUILTIN`)                   | Praktisch für erste Tests.                  |
| Schnittstellen     | UART, SPI, I²C (im ATmega integriert) | Über Pins herausgeführt.                    |

---

### 3. Pin-Überblick (für späteres Programmieren)

**Versorgung & Referenz**

* **VIN**: 7–12 V Eingang (vor Regler).
* **5V**: geregelte 5 V-Versorgung; kann Board speisen oder Lasten versorgen.
* **3.3V**: 3,3 V-Reglerausgang (bis ~50 mA).
* **GND**: mehrere Massepins.
* **RESET**: aktives Low, setzt den ATmega328P zurück.
* **AREF**: externe Referenzspannung für den ADC (optional).

**Digitale Pins D0–D13**

* **D0 (RX), D1 (TX)**: Hardware-UART – wichtig für USB-Serial und Kommunikation mit anderen Geräten.
* **D2, D3**: externe Interrupts `INT0`, `INT1` → z.B. für Sensor-Flanken.
* **D3, D5, D6, D9, D10, D11**: PWM-Pins (`analogWrite`).
* **D10–D13**: SPI (`SS`, `MOSI`, `MISO`, `SCK`) – auch für viele Shields genutzt.
* **D13**: Onboard-LED.

**Analoge Pins A0–A5**

* **A0–A5**: 10-bit-ADC-Eingänge (`analogRead`), können zusätzlich als digitale Pins D14–D19 genutzt werden.
* **A4 (SDA), A5 (SCL)**: I²C-Busleitungen.

Damit hast du alle „Programmier-relevanten“ Pins: UART (D0/D1), SPI (D10–D13), I²C (A4/A5), PWM-Ausgänge, ADC-Eingänge.

---

### 4. Betriebsmodi (Energie & Takt aus Sicht des ATmega328P)

Der ATmega328P kennt mehrere **Sleep-Modi**, die du später in C/C++ nutzen kannst, um Strom zu sparen:

* **Idle**: CPU-Takt aus, Peripherie-Takte (Timer, UART, I²C, SPI) laufen weiter.
* **ADC Noise Reduction**: wie Idle, aber fast alle Takte aus, nur ADC (und asynchroner Timer) laufen → saubere ADC-Messungen.
* **Power-save**: nur asynchroner Timer (z. B. RTC) aktiv.
* **Power-down**: fast alles aus, Weckquellen: externe Interrupts, Watchdog.
* **Standby / Extended Standby**: wie Power-down, aber Quarz weiter aktiv → sehr schneller Wake-up.

Zusätzlich:

* **Brown-Out-Detector (BOD)**: überwacht Vcc und löst Reset aus, wenn die Spannung zu weit absinkt → schützt Flash/EEPROM vor Korruption.
* **Watchdog-Timer**: eigener Oszillator, kann periodisch Reset auslösen, wenn dein Programm hängen bleibt.

Für dich beim Programmieren später wichtig:

* Normal beginst du einfach mit `setup()`/`loop()`, der Bootloader und die Board-Elektronik erledigen Reset und Takt.
* Für Low-Power-Anwendungen kannst du gezielt einen der Sleep-Modi wählen und über Interrupts (z. B. Pin-Change, Timer, UART) wieder aufwachen.


## SunFounder R3 Board – technische Daten

| Parameter                     | Wert                                   | Kommentar                                                |
|-------------------------------|----------------------------------------|----------------------------------------------------------|
| Mikrocontroller               | ATmega328P                             | UNO-kompatibler 8-bit AVR                                |
| Betriebsspannung             | 5 V                                    | Logik- und I/O-Spannung                                  |
| Eingangsspannung (empfohlen) | 7 … 12 V                               | am DC-Jack                                               |
| Eingangsspannung (Limit)     | 6 … 20 V                               | absolute Grenzen laut Hersteller                         |
| Digitale I/O-Pins            | 14 (D0–D13)                            | davon 6 PWM-fähige Pins                                  |
| PWM-Pins                     | 6 (D3, D5, D6, D9, D10, D11)           | `analogWrite()`                                          |
| Analoge Eingänge             | 6 (A0–A5)                              | 10-bit ADC                                               |
| Max. DC-Strom pro I/O-Pin    | 20 mA                                  | empfohlene Obergrenze                                    |
| Max. DC-Strom 3,3-V-Pin      | 50 mA                                  | aus Onboard-Regler                                       |
| Flash-Speicher               | 32 KB                                  | ca. 0,5 KB vom Bootloader belegt                         |
| SRAM                         | 2 KB                                   | Daten-/Stackspeicher                                     |
| EEPROM                       | 1 KB                                   | nichtflüchtiger Speicher                                |
| Taktfrequenz                 | 16 MHz                                 | Quarzoszillator                                          |
| Onboard-LED                  | Pin 13                                 | `LED_BUILTIN`                                            |
| Abmessungen                  | 68,6 mm × 53,4 mm                      | Länge × Breite                                           |
| Gewicht                      | 25 g                                   | ohne Shield                                              |
| I²C-Schnittstelle            | A4 (SDA), A5 (SCL)                     | auch am 2-poligen I²C-Header herausgeführt               |
