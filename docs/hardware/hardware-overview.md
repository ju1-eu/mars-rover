---
title: "Galaxy RVR – Hardware Overview"
file: "hardware_overview.md"
version: "v0.1.0"
author: "Jan Unger"
status: "todo"      # oder: doing / review / done / blocked
kanban.board: "Mars Rover"
---

# Galaxy RVR – Hardware Overview

Kurzer Überblick über die Hardware-Plattform des Galaxy RVR:
Steuerung (SunFounder R3 + GalaxyRVR Shield), Sensorik, Aktorik und Energiesystem.

---

## 1. Systemüberblick

- **Steuerungskern**
  - SunFounder R3 Board (Arduino-UNO-kompatibles Board mit ATmega328P).
  - GalaxyRVR Shield als „Breakout“ für Motoren, Sensoren, RGB-Strip, ESP32-CAM und Energieversorgung.

- **Sensorik**
  - Ultrasonic Module (SONAR) für Distanzmessung nach vorne.
  - IR-Hindernisvermeidungsmodule links/rechts (LEFT IR / RIGHT IR) für kurze Distanzen.
  - ESP32-CAM (Kamera) für Bild/Video + WLAN.

- **Aktorik**
  - 6 TT-Getriebemotoren als Fahrantrieb (6WD, Differential Drive: linke/rechte Seite separat).
  - 4 RGB-LED-Streifen als Status-/Ambient-Licht.
  - Servo zur Schwenkung der Kamera.

- **Energiesystem**
  - 2× 18650-Li-Ion-Akkus im Battery-Pack (über Shield eingebunden).
  - Solarpanel am SOLAR-Port des Shields zur Nachladung im Betrieb.
  - Integrierter 5-V-Wandler und Ladeelektronik auf dem Shield.

---

## 2. Komponenten – Kurzbeschreibung

### 2.1 SunFounder R3 Board

- UNO-kompatibles Board mit ATmega328P.
- Stellt die Hauptsteuerlogik (Sketch) bereit.
- Kommuniziert mit Sensoren/Aktoren über das GalaxyRVR Shield.

### 2.2 GalaxyRVR Shield

- Aufsteck-Board auf den R3.
- Enthält:
  - Motor-Treiber mit zwei Kanälen (Left/Right), je Kanal bis zu 3 TT-Motoren parallel.
  - Ports für SONAR, CAMERA, LEFT/RIGHT IR, RGB-Strip, Servo.
  - Batterie- und Solar-Anschluss.
  - 5-V-Stromversorgung und Ladeelektronik.
  - Status-LEDs (Power, Charging, Battery-Level).

### 2.3 ESP32-CAM + Kamera-Adapterplatine

- **ESP32-CAM**:
  - ESP32-Modul mit OV2640-Kamera.
  - Stellt WLAN-Verbindung und Videostream zur App bereit.
- **Kamera-Adapterplatine**:
  - Verbindung zwischen CAMERA-Port am Shield und ESP32-CAM.
  - Bricht 5 V, GND und Steuer-/UART-Signale auf passende Pins auf.
  - **HINWEIS**: QMC6310 – 3-Achsen-Magnetfeldsensor nicht verbaut im Mars-Rover

### 2.4 Ultrasonic Module (SONAR)

- Klassisches HC-SR04-artiges Ultraschallmodul.
- Misst Entfernungen im Bereich weniger Zentimeter bis einige Meter.
- TRIG/ECHO-Signal über SONAR-Port mit dem R3 verbunden.

### 2.5 IR-Hindernisvermeidungsmodul (LEFT/RIGHT IR)

- Zwei IR-Sensoren links und rechts am Chassis.
- Detektieren reflektiertes IR-Licht (Hindernis im Nahbereich).
- Digitaler Ausgang:
  - Typisch: LOW = Hindernis erkannt, HIGH = frei.

### 2.6 4 RGB-LED-Streifen (RGB Strip)

- Vier einzelne RGB-Streifen am Chassis.
- Jeder Streifen mit R/G/B-Kanälen, ansteuerbar per PWM.
- Verwendung:
  - Richtungs-/Statusanzeigen.
  - Energiesparende Dimmung durch PWM (z. B. 30 % Helligkeit).

### 2.7 Servo (Kamera-Servo)

- Kleiner 9-g-Servo (Standard-Servo).
- Schwenkt die Kamera vertikal.
- Ansteuerung über PWM-Signal (typ. 50 Hz, 1–2 ms Puls).

### 2.8 TT-Motoren (Motor Port)

- Insgesamt 6 TT-Getriebemotoren als Fahrantrieb (6WD).
- Auf dem Shield zwei Motor-Kanäle:
  - **Left**: bis zu 3 Motoren parallel an einem H-Brückenkanal.
  - **Right**: bis zu 3 Motoren parallel am zweiten H-Brückenkanal.
- Werden über H-Brücke auf dem Shield angesteuert.
- Differenzialantrieb: linke und rechte Seite separat drehzahl- und richtungsgeregelt.

### 2.9 Solarpanel

- 6-V-Solarpanel mit ca. 4 W Nennleistung.
- Wird an SOLAR-Port des Shields angeschlossen.
- Dient zur Nachladung des 18650-Battery-Packs über die integrierte Ladeelektronik.

### 2.10 18650-Batteriepack

- Zwei 18650-Li-Ion-Zellen in Serie.
- Das GalaxyRVR Shield stellt DC/DC-Wandler auf 5 V und Ladecontroller bereit.
- Versorgt GalaxyRVR Shield, R3 Board und Peripherie.

---

## 3. Technische Übersicht (Listenform)

### 3.1 SunFounder R3 Board

- **Hauptfunktion**
  - Mikrocontroller-Plattform (ATmega328P).
  - Läuft den Haupt-Sketch (z. B. Robotik-Logik, Diagnosen, Kommunikation).

- **Versorgung / Leistung**
  - 5 V über USB oder über Shield.
  - Eingangsspannung an Barrel-Jack 7–12 V (bei Standalone-Betrieb ohne Shield).

- **Schnittstellen / Ports**
  - 14 digitale I/O-Pins, davon 6 PWM-Pins.
  - 6 analoge Eingänge (A0–A5).
  - UART, SPI, I²C über Standard-UNO-Pinout.

- **Wichtige technische Daten / Hinweise**
  - Taktfrequenz: 16 MHz.
  - Flash: 32 KB (davon 0,5 KB für Bootloader).
  - SRAM: 2 KB, EEPROM: 1 KB.
  - Wird über USB oder ICSP programmiert.

---

### 3.2 GalaxyRVR Shield

- **Hauptfunktion**
  - Zentraler Knoten für Motorsteuerung, Sensoranschlüsse und Energieverteilung.
  - Integration von Akku, Solarpanel und 5-V-Verteilung.

- **Versorgung / Leistung**
  - Eingang: 18650-Battery-Pack (ca. 6,6–8,4 V intern).
  - 5-V-Ausgangsrail für R3 Board, ESP32-CAM, Sensoren, Aktoren.
  - Solar-Eingang 6 V / 660 mA (max. Panel).

- **Schnittstellen / Ports**
  - MOTOR-Port (Left/Right) für insgesamt 6 TT-Motoren (je 3 pro Kanal).
  - SONAR-Port für Ultrasonic-Modul.
  - LEFT/RIGHT IR-Ports für IR-Sensoren.
  - RGB-Port für 4 RGB-LED-Streifen.
  - CAMERA-Port für Kamera-Adapterplatine/ESP32-CAM.
  - SOLAR-Port und BAT-Port für Energie.

- **Wichtige technische Daten / Hinweise**
  - Integrierte H-Brücke (2 Kanäle Left/Right).
  - Ladeelektronik mit Status-LEDs (Power, Charge, Battery-Level).
  - Spannung/Ströme müssen in der Praxis durch Messung verifiziert werden.

---

### 3.3 Ultrasonic Module (SONAR)

- **Hauptfunktion**
  - Distanzmessung vor dem Rover.
  - Erkennung von Hindernissen mit größerer Reichweite als IR-Module.

- **Versorgung / Leistung**
  - Betriebsspannung: 5 V.
  - Stromaufnahme: typ. < 20 mA.

- **Schnittstellen / Ports**
  - SONAR-Port am Shield:
    - VCC, GND, TRIG, ECHO.

- **Wichtige technische Daten / Hinweise**
  - Messbereich: typ. ~2–400 cm (modellabhängig).
  - Auflösung im Millimeterbereich.
  - Messung über Pulsbreite (pulseIn).

---

### 3.4 IR-Hindernisvermeidungsmodul (LEFT/RIGHT IR)

- **Hauptfunktion**
  - Nahbereichs-Hinderniserkennung (z. B. Kante, Wand).
  - Ergänzt den SONAR im Very-Near-Field.

- **Versorgung / Leistung**
  - Betriebsspannung: 3,3–5 V (typ. 5 V am Shield).
  - Stromaufnahme: wenige Milliampere.

- **Schnittstellen / Ports**
  - LEFT IR / RIGHT IR Ports:
    - 3 Pins: VCC, GND, SIG.

- **Wichtige technische Daten / Hinweise**
  - Digitaler Ausgang:
    - LOW = Hindernis erkannt.
    - HIGH = frei.
  - Empfindlichkeit über Potentiometer am Modul einstellbar.

---

### 3.5 ESP32-CAM + Kamera-Adapterplatine

- **Hauptfunktion**
  - Bild-/Videoaufnahme.
  - WiFi-Streaming zur Smartphone-/Tablet-App.
  - Zusätzlich Rechenleistung (ESP32) für Bildverarbeitung möglich.

- **Versorgung / Leistung**
  - 5 V Versorgung über Kamera-Adapterplatine (vom Shield).
  - Stromaufnahme:
    - Stand: ca. 150–200 mA.
    - Spitzen (WiFi + Kamera aktiv): > 250 mA.

- **Schnittstellen / Ports**
  - Verbindung zum Shield über CAMERA-Port.
  - UART/Serielle Verbindung zum R3 bzw. zur App (je nach Setup).
  - **HINWEIS**: QMC6310 – 3-Achsen-Magnetfeldsensor nicht verbaut im Mars-Rover

- **Wichtige technische Daten / Hinweise**
  - Kamera: OV2640 (2 MP).
  - ESP32-Dual-Core, 2,4 GHz WiFi.
  - Firmware/Beispielcode von SunFounder für App-Integration.

---

### 3.6 4 RGB-LED-Streifen (RGB Strip)

- **Hauptfunktion**
  - Optische Rückmeldung (Status, Fahrtrichtung, „Ambilight“).
  - Visualisierung von Betriebsmodi (z. B. Energiesparen, Warnung).

- **Versorgung / Leistung**
  - Betriebsspannung: 5 V.
  - Stromaufnahme (Schätzung):
    - Volle Helligkeit: ca. 150 mA je Streifen.
    - 30 %-PWM: ca. 45 mA je Streifen.

- **Schnittstellen / Ports**
  - RGB-Port am Shield:
    - R, G, B-Kanal je Streifen + 5 V + GND (Streifen parallel geschaltet).
  - Ansteuerung über PWM-Pins am R3 (via Shield).

- **Wichtige technische Daten / Hinweise**
  - Helligkeit fein per PWM dimmbar (SoftPWM-Treiber in deinem Projekt).
  - Energiesparmodus: z. B. 30 % Helligkeit spart ca. 2/3 LED-Leistung.

---

### 3.7 Servo (Kamera-Servo)

- **Hauptfunktion**
  - Vertikales Schwenken der Kamera.
  - Blickwinkel-Anpassung während der Fahrt.

- **Versorgung / Leistung**
  - Betriebsspannung: 4,8–6 V.
  - Stromaufnahme:
    - Im Stillstand: niedrig.
    - Unter Last/Bewegung: kurzzeitig bis einige 100 mA.

- **Schnittstellen / Ports**
  - Servo-Port am Shield:
    - PWM-Signal, VCC, GND (typ. an D6).

- **Wichtige technische Daten / Hinweise**
  - Stellbereich: typ. 0–180°.
  - Typische Impulsbreite: 1 ms (Min) bis 2 ms (Max) bei 50 Hz.

---

### 3.8 TT-Motoren (6×, Motor Port)

- **Hauptfunktion**
  - Fahrantrieb des Rovers (6WD, zwei Motorgruppen links/rechts).
  - Umsetzt Drehmoment auf die Räder.

- **Versorgung / Leistung**
  - Nennspannung: 3–6 V.
  - Stromaufnahme:
    - Leerlauf: ca. 100–200 mA pro Motor.
    - Unter Last: einige 100 mA.
    - Stall: > 1 A möglich (vorsichtig dimensionieren).

- **Schnittstellen / Ports**
  - Motor-Port am Shield:
    - Je Kanal zwei Anschlüsse (Motor A / Motor B), daran je bis zu 3 Motoren parallel.
  - Ansteuerung mittels H-Brücke auf dem Shield (Richtung + PWM).

- **Wichtige technische Daten / Hinweise**
  - Getriebeübersetzung z. B. 1:120 (modellabhängig).
  - Drehzahl- und Drehmomentbereich reichen für Indoor-Robotik.
  - PWM-„Cruise“-Bereich (ca. 30–60 % von SpeedMax) spart Akku und schont Getriebe.

---

### 3.9 Solarpanel

- **Hauptfunktion**
  - Nachladen des 18650-Battery-Packs im Betrieb.
  - Verlängern der Betriebsdauer im Outdoor-Einsatz.

- **Versorgung / Leistung**
  - Nennspannung: 6 V.
  - Nennstrom: 660 mA (Peak bei voller Sonne).
  - Nennleistung: ca. 4 W.

- **Schnittstellen / Ports**
  - SOLAR-Port am Shield:
    - 2-Pin (VCC, GND).

- **Wichtige technische Daten / Hinweise**
  - Ladezeit voll: theoretisch ~7,2 h bei optimalem Sonnenlicht.
  - In der Praxis stark abhängig von Einstrahlung, Winkel, Temperatur.
  - Lädt über denselben Ladeweg wie USB-C.

---

### 3.10 18650-Batteriepack

- **Hauptfunktion**
  - Primäre Energiequelle des Rovers.
  - Versorgt 5-V-Rail über DC/DC-Wandler auf dem Shield.

- **Versorgung / Leistung**
  - 2× 18650-Zellen (in Serie):
    - Nennspannung: ~7,4 V (3,7 V pro Zelle).
    - Kapazität: z. B. 2000 mAh pro Zelle.
  - Energieinhalt grob:
    - \(E \approx 2 \cdot 3{,}7\,\mathrm{V} \cdot 2{,}0\,\mathrm{Ah} \approx 14{,}8\,\mathrm{Wh}\).

- **Schnittstellen / Ports**
  - BAT-Port am Shield (z. B. XH2.54-3P).
  - USB-C zum Laden (5 V / 2 A).
  - SOLAR-Port als zusätzliche Ladequelle.

- **Wichtige technische Daten / Hinweise**
  - Laufzeit-Angabe von SunFounder: ~90–130 min (je nach Last).
  - Ladedauer via USB-C: ca. 130 min.
  - Laden über Solarpanel: deutlich länger, stark lichtabhängig.
  - Batteriespannung (z. B. 6,6–8,4 V) kann per ADC am R3 überwacht werden.
