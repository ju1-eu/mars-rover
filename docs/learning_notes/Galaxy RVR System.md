# Technische Spezifikation: Galaxy RVR System

Dieser Überblick fasst die technischen Daten der Hardware-Komponenten sowie die Logik der Pin-Belegung zusammen.

## 1. Technische Daten der Komponenten

Die Kernkomponenten sind nach ihrer funktionalen Rolle im System gegliedert.

### A. Steuerung & Logik

* **SunFounder R3 Board**
    * **Typ:** Arduino Uno R3 Klon
    * **Mikrocontroller:** ATmega328P
    * **Takt:** 16 MHz
    * **Logikspannung:** $V_{logic} = 5\text{V}$
    * **Rolle:** Hauptsteuergerät ("Gehirn"), führt den C++ Code aus.


![Arduino Uno R3 pinout](./Arduino-Uno-R3.png)


* **GalaxyRVR Shield**
    * **Funktion:** Erweiterungsplatine (Hub) für das Kabelmanagement.
    * **Features:** Integrierte Motortreiber, Spannungswandler (Batterie zu 5V), dedizierte Ports für Sensoren (XH2.54 Anschlüsse), Ein/Aus-Schalter.
    * **Verbindung:** Steckt direkt auf dem R3 Board (Shield-Prinzip).

* **ESP32 CAM**
    * **Modul:** ESP32-S (WiFi + Bluetooth SoC).
    * **Kamera:** OV2640 (2 Megapixel).
    * **Rolle:** Video-Streaming (Webserver), Fernsteuerung via WLAN.
    * **Kommunikation:** Seriell (UART) zum R3 Board.

### B. Sensorik & Wahrnehmung

* **Ultraschallmodul (HC-SR04)**
    * **Reichweite:** $2\text{cm} \dots 400\text{cm}$
    * **Erfassungswinkel:** $\approx 15^\circ$
    * **Funktionsweise:** Die Distanz $d$ berechnet sich aus der Laufzeit $t$ des Echos und der Schallgeschwindigkeit $c$ (ca. $343 \frac{m}{s}$):
        $$d = \frac{t \cdot c}{2}$$
    * **Anwendung:** Kollisionsvermeidung und "Follow Me"-Modus.


* **IR-Hindernisvermeidung**
    * **Typ:** Infrarot-Reflexionssensoren (2 Einheiten: Links/Rechts).
    * **Ausgabe:** Digitales Signal (HIGH/LOW).
    * **Funktion:** Nahbereichserkennung (z. B. Wand direkt vor dem Rad) und Absturzsicherung.

### C. Aktorik (Bewegung & Licht)

* **TT-Motoren (6WD)**
    * **Anzahl:** 6 Stück DC-Getriebemotoren.
    * **Übersetzung:** $i = 1:48$
    * **Spannung:** $3\text{V} \dots 6\text{V}$
    * **Besonderheit:** Hohes Drehmoment, keine Encoder (Open-Loop-Steuerung).

* **Servo**
    * **Typ:** Micro Servo (SG90 Baugröße).
    * **Winkelbereich:** $0^\circ \dots 180^\circ$
    * **Funktion:** Tilt-Mechanismus (Kippen) für die Kamera/Ultraschall-Einheit.

* **RGB-LED-Streifen**
    * **Typ:** WS2812 (Neopixel).
    * **Anzahl:** 4 Streifen (Unterbodenbeleuchtung).
    * **Steuerung:** Adressierbar über einen einzigen Daten-Pin (One-Wire-Protocol).

### D. Energieversorgung

* **18650 Batterie**
    * **Konfiguration:** 2x Li-Ion Zellen in Serie.
    * **Systemspannung:** $V_{sys} \approx 2 \cdot 3.7\text{V} = 7.4\text{V}$
    * **Kapazität:** Typisch $2000\text{mAh} \dots 3000\text{mAh}$.
    * **Rolle:** Hauptstromquelle für Motoren (High Power) und Logik (via Step-Down).

* **Solarpanel**
    * **Spannung (Leerlauf):** $5\text{V} \dots 6\text{V}$
    * **Leistung:** Gering (Erhaltungsladung).
    * **Funktion:** Didaktisches Element für regenerative Energien; lädt den Akku bei starker Einstrahlung minimal nach.

---

## 2. Pin-Belegung (Mapping Tabelle)

Die folgende Tabelle definiert die Schnittstellen zwischen dem Arduino R3 und den Komponenten auf dem GalaxyRVR Shield.

> **Hinweis:** Die Kommunikation zwischen Arduino und ESP32 erfolgt über **SoftwareSerial**, um den Hardware-UART (Pins 0/1) für das USB-Debugging freizuhalten.

| Arduino Pin | Funktion / Komponente | Signaltyp | Beschreibung |
| :--- | :--- | :--- | :--- |
| **D2** | Software Serial (RX) | Digital | Empfang vom ESP32 CAM (App-Befehle). |
| **D3** | Software Serial (TX) | Digital | Senden an ESP32 CAM. |
| **D4** | IR Sensor Links | Digital In | Hinderniserkennung links (LOW/HIGH). |
| **D5** | IR Sensor Rechts | Digital In | Hinderniserkennung rechts (LOW/HIGH). |
| **D6 – D9*** | Motortreiber | PWM / Digital | Steuerung der 6 Motoren (Gruppiert in Links/Rechts). |
| **D10** | Servo | PWM | Steuersignal für den Kamerawinkel (Tilt). |
| **D11** | RGB LED (WS2812) | Digital Out | Datenleitung für alle 4 LED-Streifen (verkettet). |
| **D12** | Ultraschall Trigger | Digital Out | Sendet den Schallimpuls ($t_{start}$). |
| **D13** | Ultraschall Echo | Digital In | Empfängt das Echo für Zeitmessung ($t_{end}$). |
| **A0 - A5** | Erweiterung | Analog In | Oft für Liniensensoren oder Batterieüberwachung genutzt. |

*\*Die Zuordnung der Motor-Pins hängt vom spezifischen Motortreiber-Chip auf dem Shield ab, wird aber softwareseitig meist als Gruppe (Left/Right) abstrahiert.*

---

## 3. Zusammenfassung der Architektur

### Signalfluss
1.  **Input:** Die App sendet Steuerbefehle via WiFi an den **ESP32**.
2.  **Bridge:** Der ESP32 leitet diese via Serial (UART) an den **Arduino R3** weiter.
3.  **Output:** Der Arduino verarbeitet die Logik und steuert Aktorik (Motoren, Servos, LEDs) über das **Shield**.

### Stromfluss
1.  **Quelle:** Die 18650 Batterien ($7.4\text{V}$) speisen das Shield.
2.  **Last (Hochstrom):** Das Shield versorgt die Motoren direkt mit der Batteriespannung.
3.  **Last (Logik):** Ein Spannungsregler (LDO/Buck-Converter) wandelt $7.4\text{V} \rightarrow 5\text{V}$ für Arduino, ESP32 und Sensoren.


---

Ja, mit den Infos zum R3-Board und zum GalaxyRVR Shield lässt sich das sauber einordnen.

### 1. Rollenverteilung: R3 vs. GalaxyRVR Shield

**SunFounder R3 Board**
– Im Prinzip ein Arduino Uno-Klon mit ATmega328P, 5-V-Logik, 6 PWM-Pins, 6 ADC-Kanälen usw.([SunFounder Dokumentation][1])
– Keine eigene Ladeelektronik für den 2s-18650-Pack, keine spezielle Solar-Logik.

**GalaxyRVR Shield**([SunFounder Dokumentation][2])
– „All-in-one“-Expansionsboard mit Ports für Motor, RGB-Strip, IR, SONAR, ESP32-CAM usw.
– Enthält das komplette **Power-/Lademanagement**:

* Battery Port (6,6–8,4 V, PH2.0-3P)
* USB-C Charge Port (5 V / 2 A), Ladedauer ca. 130 min
* **Solar-Port** (XH2.54-2P), das Panel lädt über den gleichen Ladecontroller
* Lade-LED („Charge Indicator“, rot)
* Power-LED (grün)
* „Battery Indicator“: zwei orange LEDs für Batteriestatus, blinken beim Laden und gehen aus, wenn der Akku leer ist.([SunFounder Dokumentation][2])

Wichtig: Die Doku beschreibt keine Verbindung dieser Status-LEDs zu einem Arduino-Pin. Der Charger ist für dich im Code **eine Blackbox**; du siehst ihn nur indirekt.

---

### 2. Kann man softwareseitig erkennen, ob das Solarpanel lädt?

Mit der **Standardhardware** (ohne Mod) gilt:

1. **Direkt: nein**, der Ladechip gibt dir kein digitales „CHG“-Signal an einen Arduino-Pin.

   * Der Charge-Status steckt in der LED-Ansteuerung auf dem Shield.
   * Die Arduino-Pins sind laut Pinout für Motor, RGB, IR, SONAR, etc. belegt, nicht für „CHG OK“.([SunFounder Dokumentation][2])

2. **Indirekt: ja**, über deine Batterie-Messung und ggf. Lastszenarien:

   * Du misst (U_\text{Bat}) über `getBatteryVoltage()` (Spannungsteiler an A3).
   * Du rechnest SOC % über `getBatteryPercentage()`.
   * Wenn bei **ähnlicher Last** (z. B. gleiches „Cruise“-Profil) die Spannung/SOC-Kurve

     * langsamer fällt als im Indoor-Vergleich, oder
     * sogar **leicht steigt**,
       ist das ein starker Hinweis, dass das Solarpanel netto Ladung in den Akku schiebt.

Das ist genau der Ansatz aus Lesson 13:
– Indoor-Run (ohne Panel-Effekt) vs. Outdoor-Run (Panel aktiv), je 30 min, SOC vergleichen.([SunFounder Dokumentation][3])

---

### 3. Was siehst du *visuell* am Shield?

Ohne Code, nur über LEDs am Shield:([SunFounder Dokumentation][2])

* Charge Indicator (rot):

  * Leuchtet, wenn über USB-C geladen wird.
  * Bei Solarladung verhält er sich analog, weil beides in denselben Ladepfad geht.
* Battery Indicator (2× orange):

  * Blinken beim Laden (egal ob USB-C oder Solar).
  * Aus, wenn Akku leer ist.
* Power Indicator (grün):

  * Zeigt an, dass der Shield/R3 über den Akku eingeschaltet ist.

Damit kannst du optisch eindeutig sehen, dass **Ladung aktiv ist** – aber ohne zusätzliche Hardware nicht per Software abfragen.

---

### 4. Was wäre möglich, wenn du mehr willst?

Wenn du in Zukunft „richtig“ wissen willst, ob geladen wird, hast du drei Optionen (größerer Eingriff):

1. **Stromsensor einbauen** (z. B. INA219 o. ä.) in die Batterieleitung:

   * dann kannst du „I > 0“ = Ladung, „I < 0“ = Entladung sauber messen.

2. **CHG-Pin des Ladechips abgreifen**, falls vorhanden:

   * er steuert meist die Charge-LED.
   * Mit einer kleinen Zusatzschaltung könntest du diesen Zustand auf einen freien Arduino-Pin legen.
   * Das erfordert Eingriff auf dem Shield → nur, wenn du wirklich basteln willst.

3. **Solar-Port-Spannung messen** (Panel-Seite) über zusätzlichen Spannungsteiler und freien ADC:

   * Damit siehst du, ob das Panel überhaupt relevante Spannung liefert (z. B. > 6 V).
   * Kombiniert mit Batteriespannung kannst du dann zwischen „Panel da, aber Akku nicht wirklich geladen“ und „Panel + Anstieg von (U_\text{Bat})“ unterscheiden.

[1]: https://docs.sunfounder.com/projects/galaxy-rvr/en/latest/hardware/cpn_r3_board.html "SunFounder R3 Board — SunFounder GalaxyRVR Kit for Arduino 1.0 documentation"
[2]: https://docs.sunfounder.com/projects/galaxy-rvr/en/latest/hardware/cpn_galaxy_shield.html "GalaxyRVR Shield — SunFounder GalaxyRVR Kit for Arduino 1.0 documentation"
[3]: https://docs.sunfounder.com/projects/galaxy-rvr/en/latest/lesson13_solar_panel.html "Lesson 13: Investigating the Mars Rover Energy System — SunFounder GalaxyRVR Kit for Arduino 1.0 documentation"
