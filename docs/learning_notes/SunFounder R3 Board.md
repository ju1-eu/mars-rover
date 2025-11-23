# Cornell-Notizen: SunFounder R3 Board (ATmega328P)

| **Fragen / Schlüsselbegriffe** | **Notizen & Technische Parameter** |
| :--- | :--- |
| **Was ist das R3 Board?** | \* Mikrocontroller-Board, basierend auf dem **ATmega328P**.<br>\* **Funktion:** Hat fast dieselben Funktionen wie das Arduino Uno und ist austauschbar.<br>\* **Betrieb:** Kann über USB-Kabel, AC-DC-Adapter oder Batterie gestartet werden. |
| **I/O-Schnittstellen** | \* **Digitale I/O-Pins:** 14 (Pins 0–13).<br>\* **PWM-Ausgänge:** 6 (Pins 3, 5, 6, 9, 10–11).<br>\* **Analoge Eingänge:** 6 (Pins A0–A5).<br>\* **I²C-Port:** A4 ($SDA$), A5 ($SCL$).<br>\* **LED\_BUILTIN:** Pin 13. |
| **Leistung & Strom** | \* **Betriebsspannung:** 5V.<br>\* **Eingangsspannung (Empfohlen):** 7–12V.<br>\* **Eingangsspannung (Limit):** 6–20V.<br>\* **DC Strom pro I/O-Pin:** 20 mA.<br>\* **DC Strom für 3.3V Pin:** 50 mA. |
| **Speicher & Takt** | \* **Mikrocontroller:** ATmega328P.<br>\* **Taktfrequenz:** 16 MHz.<br>\* **Flash-Speicher (Code):** 32 KB (davon 0,5 KB durch Bootloader verwendet).<br>\* **SRAM (Variablen):** 2 KB.<br>\* **EEPROM (Daten):** 1 KB. |

-----

### Zusammenfassung

Das **SunFounder R3 Board** ist das Herzstück des Rovers und funktional identisch mit dem Arduino Uno. Es nutzt den **ATmega328P** als Mikrocontroller und arbeitet mit einer **Betriebsspannung von 5V** bei **16 MHz** Taktfrequenz. Es bietet 14 digitale (davon 6 PWM) und 6 analoge Pins zur Steuerung. Der interne Speicher ist auf 32 KB Flash, 2 KB SRAM und 1 KB EEPROM begrenzt, was bei der Programmierung des Rovers berücksichtigt werden muss.


