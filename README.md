---
title: "GalaxyRVR Profi – Firmware-Dokumentation"
file: "README.md"
version: "v0.2.0"
author: "Jan Unger"
status: "active"
kanban.board: "Mars Rover"
---

# GalaxyRVR Profi – Firmware-Dokumentation

[GalaxyRVR - SunFounder Mars Rover Kit](https://docs.sunfounder.com/projects/galaxy-rvr/de/latest/index.html)

Quellcode download: <https://github.com/sunfounder/galaxy-rvr>

Dokumentation <https://github.com/sunfounder/galaxy-rvr/blob/docs-de/docs/source/index.rst>

Arduino Language Reference <https://www.arduino.cc/reference/en/>

## 1. Projektübersicht

Dies ist eine professionell strukturierte Firmware-Entwicklungsumgebung für zwei parallele Plattformen. Das Projekt verzichtet auf monolithischen „Spaghetti-Code“ und implementiert stattdessen eine **Schichten-Architektur (Layered Architecture)**, moderne C++-Standards und **Non-Blocking I/O**.

### Ziel-Plattformen

1. **SunFounder Galaxy RVR**
   - **Hardware:** Arduino Uno R3 (ATmega328P) + Galaxy RVR Shield
   - **Sensorik:** Ultraschall, IR, IMU (MPU6050 via I2C)
   - **Standard:** C++17 (via `-std=gnu++17`)

2. **Seeed Studio XIAO ESP32S3**
   - **Hardware:** ESP32-S3 (Dual-Core Xtensa LX7)
   - **Fokus:** Experimentierplattform für High-Level-C++-Features
   - **Standard:** C++20 (via `-std=gnu++2a`)

---

## 2. Architektur-Konzept

Das System folgt dem Prinzip der **Separation of Concerns**. Die Firmware ist in drei horizontale Schichten unterteilt.

### Schicht 1: HAL (Hardware Abstraction Layer)

- **Verzeichnis:** `src/hal/`
- **Aufgabe:** Physischer Zugriff auf die Hardware (Pins, PWM-Register, ADC, I2C-Bus).
- **Besonderheit:** Integriert Sensor-Fusion (Komplementärfilter) für die IMU direkt im Treiber, um stabile Winkel (Pitch/Roll) bereitzustellen.
- **Schnittstelle:** Stellt abstrakte Methoden wie `setSpeed(int left, int right)` oder `getPitch()` bereit.

### Schicht 2: Logik & Verhalten (Logic Layer)

- **Verzeichnis:** `src/logic/`
- **Aufgabe:** Berechnung von Bewegungsvektoren und Regelkreisen (z.B. DriveAssistant).
- **Mathematik:** Hier werden abstrakte Befehle in konkrete Motorwerte umgerechnet.

*Beispiel Kurvenfahrt:*
Bei einer Geschwindigkeit $v \in [0, 255]$ und einem Kurvenverhältnis $r \in [0.0, 1.0]$ berechnet sich die Geschwindigkeit des kurveninneren Rades $v_\text{in}$ wie folgt:

$$
v_\text{in} = v \cdot (1.0 - r)
$$

- **Abhängigkeit:** Kennt nur die HAL-Schnittstellen, keine konkreten Pin-Nummern.

### Schicht 3: Anwendung (Application Layer)

- **Verzeichnis:** `src/main.cpp`
- **Aufgabe:** High-Level-Steuerung durch einen **Endlichen Automaten (Finite State Machine, FSM)**.
- **Timing:** Nutzt `millis()` für nicht-blockierende Zeitsteuerung nach dem Schema:

$$
t_\text{now} - t_\text{last} > \Delta t
$$

---

## 3. Ordnerstruktur

Die Struktur ist für **PlatformIO** optimiert und trennt öffentliche Schnittstellen (`include`), Implementierungen (`src`), Dokumentation (`docs`) und Tests (`test`).

```text
GalaxyRVR_Profi/
├── docs/
│   ├── archive/            # Alter Code (z. B. Lektion04_MotorTest.cpp)
│   ├── datasheets/         # Hardware-Datenblätter & Schaltpläne
│   ├── doxygen/            # Generierte API-Dokumentation (HTML)
│   └── learning_notes/     # Lern-Notizen & Konzepte (.md)
├── include/                # Globale Header (Konfiguration & Schnittstellen)
│   ├── Config.h            # Systemweite Konstanten (constexpr)
│   ├── Motion.h            # Schnittstelle für Bewegungslogik
│   └── Pins.h              # Pin-Mapping für den Galaxy RVR
├── src/                    # Quellcode
│   ├── hal/                # Hardware-Treiber (Motor, Sensor, Actuator)
│   ├── logic/              # Plattformunabhängige Algorithmen
│   └── main.cpp            # Einstiegspunkt & State Machine (FSM)
├── test/                   # Experimenteller Code / Unit-Tests
├── platformio.ini          # Build-Konfiguration & Environments
├── README.md               # Diese Dokumentation
└── LICENSE.md              # Lizenz (z. B. MIT)
````

-----

## 4\. Konfiguration & Hardware (Galaxy RVR)

### Pin-Mapping

Das Pinning ist in `include/Pins.h` als `constexpr` definiert. Aufgrund der Hardware-Limitierung des ATmega328P (nur 6 Hardware-PWM-Kanäle) wird für bestimmte Pins eine Software-PWM-Lösung benötigt.

| Funktion                 | Arduino-Pin | Typ     | Besonderheit       |
| ------------------------ | ----------: | ------- | ------------------ |
| Motor links (Vorwärts)   |           2 | Digital | Benötigt `SoftPWM` |
| Motor links (Rückwärts)  |           3 | PWM     | Hardware-PWM       |
| Motor rechts (Rückwärts) |           4 | Digital | Benötigt `SoftPWM` |
| Motor rechts (Vorwärts)  |           5 | PWM     | Hardware-PWM       |
| **IMU (SDA)** |          A4 | I2C     | MPU-6050 / GY-521  |
| **IMU (SCL)** |          A5 | I2C     | MPU-6050 / GY-521  |

Weitere Pins (Sensoren, LEDs etc.) werden zentral in `Pins.h` gepflegt, um magische Zahlen im Code zu vermeiden.

### Mathematische Modelle

In `src/logic/Motion.cpp` wird die differentielle Lenkung berechnet. Die PWM-Werte $PWM_L$ und $PWM_R$ ergeben sich aus der Soll-Geschwindigkeit $v$ und einem Lenk-Faktor.

  * Geradeausfahrt: $PWM_L = v,\quad PWM_R = v$
  * Panzer-Wende: $PWM_L = -v,\quad PWM_R = +v$

-----

## 5\. Coding Guidelines

Wir folgen strikten Richtlinien für **Embedded C++**:

1.  **Non-Blocking I/O**
    Kein Einsatz von `delay()` im Hauptloop. Alle zeitabhängigen Vorgänge werden über `millis()` und Zustandsautomaten (FSM) umgesetzt. Ausnahme: Kurze Wartezeiten bei der Initialisierung (Setup).

2.  **Starke Typisierung**
    Verwendung von `enum class` für Zustände und Modusvariablen, um implizite Casts zu `int` zu vermeiden.

3.  **Compiler-Standards**

      * AVR (Uno): `-std=gnu++17` (C++17 mit GNU-Extensions)
      * ESP32 (XIAO): `-std=gnu++2a` (C++20-Support)

4.  **Ressourcen-Management**

      * Einsatz von `constexpr` für Konstanten, um RAM-Verbrauch zur Laufzeit zu minimieren.
      * Vermeidung dynamischer Speicherallokation (`new`/`delete`) im zeitkritischen Pfad.

5.  **Fehlerbehandlung / Logging**

      * Klare Trennung zwischen produktivem Code und Debug-Meldungen.
      * Einsatz von `F("...")` Strings beim AVR, um RAM zu sparen.

-----

## 6\. Build-Umgebungen (Environments)

Das Projekt wird über `platformio.ini` gesteuert und nutzt getrennte **Release-** und **Debug-Umgebungen**.

### Umgebung A: `env:uno` (Galaxy RVR – Release)

  * **Plattform:** Atmel AVR
  * **Board:** Arduino Uno
  * **Code:** Kompiliert `src/hal/` und `src/logic/`, nutzt `SoftPWM` und `Wire`.
  * **Port:** Automatisch oder via `/dev/cu.usbserial-xxxx`
  * **Einsatz:** Produktions-Firmware.

### Umgebung B: `env:uno-debug` (Galaxy RVR – Debug)

  * **Basis:** Erweitert `env:uno`
  * **Zusatz:** `-DDEBUG` für zusätzliche Log-Ausgaben.
  * **Zweck:** Fehlersuche, Diagnostik.

-----

## 7\. Workflow für Erweiterungen

Beispiel: Ein neuer Sensor (z. B. IR-Linienfolger) soll integriert werden.

1.  **Hardware-Konfiguration**
    Pin in `include/Pins.h` als `constexpr` definieren.

2.  **Treiber (HAL)**
    Klasse `IrSensor` in `src/hal/` implementieren.

3.  **Logik (Behavior)**
    Auswertung der Sensordaten in `src/logic/` implementieren.

4.  **Integration in die FSM**
    In `src/main.cpp` einen neuen Zustand ergänzen und Übergänge definieren.

-----

*Dokumentation aktualisiert am: 24.11.2025*

