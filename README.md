---
title: "GalaxyRVR Profi – Firmware-Dokumentation"
file: "README.md"
version: "1.2.0"
author: "Jan Unger"
status: "active"
kanban.board: "Mars Rover"
---

# GalaxyRVR Profi – Firmware-Dokumentation

Diese Dokumentation beschreibt die professionelle Firmware-Architektur für den SunFounder Galaxy RVR.
Das Projekt dient als Referenzimplementierung für Embedded-C++-Entwicklung, mit Fokus auf Modularität, Wartbarkeit und deterministisches Zeitverhalten.

## Links

- [SunFounder Galaxy RVR Kit](https://www.sunfounder.com/products/galaxy-rvr-kit) (Beispiel-Link)
- [API-Dokumentation (Doxygen)](./docs/doxygen/index.html)
- [Projekt-Board & Roadmap](./docs/ROADMAP.md)

---

## 1. Projektphilosophie & Zielsetzung

Herkömmliche Maker-Projekte leiden häufig unter monolithischem „Spaghetti-Code“, der Hardwarezugriffe, Logik und Timing vermischt. Dieses Projekt verfolgt einen industriellen Ansatz:

- **Schichten-Architektur (Layered Architecture):**
  Strikte Trennung von Hardware-Treibern (HAL) und Verhaltenslogik.

- **Deterministisches Timing:**
  Verzicht auf blockierende `delay()`-Aufrufe zugunsten von nicht-blockierenden Zustandsautomaten (Finite State Machines).

- **Doppelte Legitimation:**
  Der Code ist so geschrieben, dass er fachlich präzise (für Experten) und gleichzeitig didaktisch transparent (für Lernende) ist.

### Ziel-Plattformen

Der Code ist plattformunabhängig konzipiert und unterstützt zwei Targets:

- **Production:** Arduino Uno R3 (ATmega328P) – Ressourcenoptimiert.
- **Experimental:** Seeed Studio XIAO ESP32S3 – Performance & modernes C++20.

---

## 2. Architektur-Konzept

Das System folgt dem Prinzip der **Separation of Concerns**.
Abhängigkeiten verlaufen ausschließlich von oben nach unten
(„High-Level modules should not depend on low-level modules“ – Dependency Inversion Principle).

### Schicht 1: HAL (Hardware Abstraction Layer)

- **Verzeichnis:** `src/hal/`
- **Verantwortung:** Kapselung der physischen Hardware.
- **Details:**
  - Abstraktion von Registerzugriffen, I²C-Kommunikation und PWM-Erzeugung.
  - Beispiel: `HAL::Motor::setSpeed(int speed)` steuert die Motoren an – unabhängig davon, ob dahinter eine H-Brücke per PWM oder ein Bus-System liegt.
  - **Sensor-Fusion:** Rohdaten der IMU (Gyroskop/Accelerometer) werden hier vorverarbeitet (z. B. Komplementärfilter), um stabile Lagewinkel bereitzustellen.

### Schicht 2: Logik & Verhalten (Logic Layer)

- **Verzeichnis:** `src/logic/`
- **Verantwortung:** Reine Algorithmen und Regelungstechnik (plattform-agnostisch).
- **Details:**
  - Übersetzung von abstrakten Wünschen („Fahre Kurve“) in konkrete Aktorwerte.
  - **Differential-Drive-Kinematik:**
    Berechnung der Raddrehzahlen basierend auf Soll-Geschwindigkeit $v$ und Kurvenradius $r$:
    $$v_{\text{in}} = v \cdot (1.0 - r)$$
  - **DriveAssistant:**
    PID-Regler, der die Gierrate (Yaw-Rate) nutzt, um den Rover aktiv auf Kurs zu halten.

### Schicht 3: Anwendung (Application Layer)

- **Verzeichnis:** `src/main.cpp`
- **Verantwortung:** High-Level-Steuerung und Zustandsmanagement.
- **Details:**
  - Implementiert die Business-Logik als endlichen Automaten (FSM).
  - Verwaltet Modi wie:
    - Hardware-Diagnose
    - Autonomes Fahren
    - Fernsteuerung
  - Stellt sicher, dass Sicherheitschecks (z. B. Not-Aus bei Kippen) zyklisch durchlaufen werden.

---

## 3. Ordnerstruktur

Die Struktur ist für PlatformIO optimiert und trennt Interface (`include`) strikt von Implementation (`src`).

```text
GalaxyRVR_Profi/
├── docs/                   # Dokumentation
│   ├── datasheets/         # Hardware-Spezifikationen
│   ├── doxygen/            # Generierte HTML-API-Doku
│   └── learning_notes/     # Fachliche Konzepte & Notizen
├── include/                # Öffentliche Schnittstellen (Header)
│   ├── Config.h            # Zentrale Systemkonfiguration (constexpr)
│   ├── Pins.h              # Hardware-Mapping
│   ├── hal/                # Treiber-Interfaces
│   └── logic/              # Logik-Interfaces
├── src/                    # Implementierung (.cpp)
│   ├── hal/                # Hardware-Spezifika (Treiber)
│   ├── logic/              # Algorithmen
│   └── main.cpp            # Einstiegspunkt & State Machine
├── test/                   # Unit-Tests & Diagnose-Sketches
└── platformio.ini          # Build-Umgebungen
```

-----

## 4\. Hardware-Konfiguration (Mapping)

Die Pin-Belegung ist zentral in `include/Pins.h` definiert.
„Magische Zahlen“ im Code werden vermieden.
Aufgrund der Timer-Limitierungen des ATmega328P wird eine Hybrid-Lösung aus Hardware- und Software-PWM genutzt.

| Aktor / Sensor | Arduino-Pin | Signal-Typ       | Anmerkung                          |
| -------------- | ----------: | ---------------- | ---------------------------------- |
| Motor links    |        2, 3 | SoftPWM / HW-PWM | Pin 2 ist Richtung/PWM via SoftPWM |
| Motor rechts   |        4, 5 | SoftPWM / HW-PWM | Pin 4 ist Richtung/PWM via SoftPWM |
| IMU (MPU6050)  |       A4,A5 | I²C              | Standard `Wire`-Library            |
| Ultraschall    |        7, 8 | Digital I/O      | Trigger / Echo                     |
| Servo (Tilt)   |          10 | PWM              | Kamera-Neigung                     |
| RGB-LEDs       |          13 | Timing-kritisch  | WS2812-Protokoll                   |

-----

## 5\. Coding Guidelines & Dokumentation

Um die Software wartbar und sicher zu halten, gelten folgende Richtlinien (siehe `CONTRIBUTING.md`).

### 5.1 Dokumentations-Stil (Doxygen)

Doxygen wird mit Custom-Tags genutzt, um nicht nur das **Wie**, sondern auch das **Warum** und Sicherheitsaspekte zu dokumentieren:

  * `@brief` – Kurze Zusammenfassung (Was tut es?).
  * `@details` – Technische Tiefe, Algorithmen, Zustandsdiagramme.
  * `@safety` – Kritische Hinweise, z. B. „Funktion stoppt Motoren bei Verbindungsabbruch“.
  * `@hardware` – Listet physische Abhängigkeiten auf.
  * `@pre` – Vorbedingungen, z. B. „Rover muss stillstehen“.

### 5.2 C++-Standards

  * **`constexpr` statt `#define`:**
    Typ-sichere Konstanten, die zur Compile-Zeit ausgewertet werden.

  * **`enum class`:**
    Verhindert implizite Typumwandlungen und erhöht die Lesbarkeit von Zustandsautomaten.

  * **Non-Blocking-Design:**
    Keine `delay()`-Aufrufe in der `loop()`. Zeitsteuerung erfolgt ausschließlich über `millis()`-Vergleiche:

    ```cpp
    if (now - lastAction > INTERVAL) {
        // ...
    }
    ```

-----

## 6\. Build-Umgebungen

Das Projekt nutzt `platformio.ini` zur Verwaltung verschiedener Build-Targets:

  * **`env:uno` (Release):**

      * Optimiert für Speicherplatz (`-Os`).
      * Debug-Logging minimiert.

  * **`env:uno-debug` (Development):**

      * Aktiviertes Serial-Logging (`-DDEBUG`).
      * Detaillierte Sensorausgaben.

  * **`env:xiao_esp32s3` (Feature-Preview):**

      * Nutzung von C++20-Features.
      * Erweitertes Memory-Limit für komplexe Algorithmen.

-----

## 7\. Erweiterungs-Workflow

Beispiel: Neuer Sensor (z. B. IR-Linienfolger) soll hinzugefügt werden.

1.  **Konfiguration:**
    Pin in `include/Pins.h` definieren.

2.  **HAL:**
    Treiberklasse in `src/hal/LineSensor.cpp` erstellen
    (Methoden: `init()`, `read()`).

3.  **Logik:**
    Algorithmus in `src/logic/LineFollower.cpp` schreiben
    (Eingabe: Sensorwerte → Ausgabe: Lenkwinkel).

4.  **Integration:**
    In `main.cpp` instanziieren und in den State-Machine-Loop einhängen.

-----

## 8\. Status

  - **Status:** Active Maintenance
  - **Phase:** 1.0 (Abgeschlossen) – Grundlegende Fahrt, Sensor-Fusion, PID-Regelung.
  - **Letztes Update:** 25.11.2025
