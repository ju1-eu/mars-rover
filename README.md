# GalaxyRVR Profi – Firmware-Dokumentation

## 1. Projektübersicht

Dies ist eine professionell strukturierte Firmware-Entwicklungsumgebung für zwei parallele Plattformen. Das Projekt verzichtet auf monolithischen „Spaghetti-Code“ und implementiert stattdessen eine **Schichten-Architektur (Layered Architecture)**, moderne C++-Standards und **Non-Blocking I/O**.

### Ziel-Plattformen

1. **SunFounder Galaxy RVR**
   - **Hardware:** Arduino Uno R3 (ATmega328P) + Galaxy RVR Shield
   - **Fokus:** Mobile Robotik, Motorsteuerung, Sensorik
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
- **Aufgabe:** Physischer Zugriff auf die Hardware (Pins, PWM-Register, ADC).
- **Implementierung:** Nutzt plattformspezifische Frameworks (z. B. `SoftPWM` für AVR).
- **Schnittstelle:** Stellt abstrakte Methoden wie `setSpeed(int left, int right)` bereit.

### Schicht 2: Logik & Verhalten (Logic Layer)

- **Verzeichnis:** `src/logic/`
- **Aufgabe:** Berechnung von Bewegungsvektoren und Regelkreisen.
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
- **Timing:** Nutzt `millis()` für nicht-blockierende Zeitsteuerung nach dem Schema

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
│   ├── doxygen/            # Generierte API-Dokumentation (HTML, ggf. weitere Formate)
│   └── learning_notes/     # Lern-Notizen & Konzepte (.md, Infografiken, Checklisten)
├── include/                # Globale Header (Konfiguration & Schnittstellen)
│   ├── Config.h            # Systemweite Konstanten (constexpr)
│   ├── Motion.h            # Schnittstelle für Bewegungslogik
│   └── Pins.h              # Pin-Mapping für den Galaxy RVR
├── src/                    # Quellcode
│   ├── hal/                # Hardware-Treiber (z. B. Motor.cpp, Sensor.cpp)
│   ├── logic/              # Plattformunabhängige Algorithmen / Bewegungslogik
│   └── main.cpp            # Einstiegspunkt & State Machine (FSM)
├── test/                   # Experimenteller Code / Unit-Tests (z. B. ESP32-Tests)
├── platformio.ini          # Build-Konfiguration & Environments
├── README.md               # Projektbeschreibung (dieses Dokument verlinken)
└── LICENSE.md              # Lizenz (z. B. MIT)
````

---

## 4. Konfiguration & Hardware (Galaxy RVR)

### Pin-Mapping

Das Pinning ist in `include/Pins.h` als `constexpr` definiert. Aufgrund der Hardware-Limitierung des ATmega328P (nur 6 Hardware-PWM-Kanäle) wird für bestimmte Pins eine Software-PWM-Lösung benötigt.

| Funktion                 | Arduino-Pin | Typ     | Besonderheit       |
| ------------------------ | ----------: | ------- | ------------------ |
| Motor links (Vorwärts)   |           2 | Digital | Benötigt `SoftPWM` |
| Motor links (Rückwärts)  |           3 | PWM     | Hardware-PWM       |
| Motor rechts (Rückwärts) |           4 | Digital | Benötigt `SoftPWM` |
| Motor rechts (Vorwärts)  |           5 | PWM     | Hardware-PWM       |

Weitere Pins (Sensoren, LEDs etc.) werden zentral in `Pins.h` gepflegt, um magische Zahlen im Code zu vermeiden.

### Mathematische Modelle

In `src/logic/Motion.cpp` wird die differentielle Lenkung berechnet. Die PWM-Werte $PWM_L$ und $PWM_R$ ergeben sich aus der Soll-Geschwindigkeit $v$ und einem Lenk-Faktor.

* Geradeausfahrt:

  $$
  PWM_L = v,\quad PWM_R = v
  $$

* Panzer-Wende („Drehung auf der Stelle“):

  $$
  PWM_L = -v,\quad PWM_R = +v
  $$

Leichte Kurven werden über Skalierung eines Rades umgesetzt (siehe Beispiel unter **Schicht 2**).

---

## 5. Coding Guidelines

Wir folgen strikten Richtlinien für **Embedded C++**:

1. **Non-Blocking I/O**
   Kein Einsatz von `delay()`. Alle zeitabhängigen Vorgänge werden über `millis()` und Zustandsautomaten (FSM) umgesetzt.

2. **Starke Typisierung**
   Verwendung von `enum class` für Zustände und Modusvariablen, um implizite Casts zu `int` zu vermeiden.

3. **Compiler-Standards**

   * AVR (Uno): `-std=gnu++17` (C++17 mit GNU-Extensions)
   * ESP32 (XIAO): `-std=gnu++2a` (C++20-Support, z. B. `std::array`, `std::vector`, `constexpr`-Funktionen)

4. **Ressourcen-Management**

   * Einsatz von `constexpr` für Konstanten, um RAM-Verbrauch zur Laufzeit zu minimieren.
   * Möglichst keine dynamische Speicherallokation (`new`/`delete`) im zeitkritischen Pfad.

5. **Fehlerbehandlung / Logging**

   * Klare Trennung zwischen produktivem Code und Debug-Meldungen.
   * Debug-Ausgaben werden über `#ifdef DEBUG` kompiliert/entfernt.

---

## 6. Build-Umgebungen (Environments)

Das Projekt wird über `platformio.ini` gesteuert und nutzt getrennte **Release-** und **Debug-Umgebungen** für beide Ziel-Plattformen.

### Umgebung A: `env:uno` (Galaxy RVR – Release)

* **Plattform:** Atmel AVR
* **Board:** Arduino Uno
* **C++-Standard:** `-std=gnu++17`
* **Code:** Kompiliert `src/hal/` und `src/logic/`, nutzt `SoftPWM` für zusätzliche PWM-Kanäle.
* **Port (Beispiel):** `/dev/cu.usbserial-xxxx` (USB-B-Kabel)
* **Einsatz:** Produktions-Firmware, Debug-Ausgaben minimal.

### Umgebung B: `env:uno-debug` (Galaxy RVR – Debug)

* **Basis:** Erweitert `env:uno`
* **Zusatz:** `build_type = debug`, `-DDEBUG` für zusätzliche Log-Ausgaben.
* **Zweck:** Fehlersuche, detaillierte Diagnostik.

### Umgebung C: `env:xiao_esp32s3` (XIAO ESP32S3 – Release)

* **Plattform:** Espressif 32
* **Board:** Seeed Studio XIAO ESP32S3
* **C++-Standard:** `-std=gnu++2a` (C++20)
* **Filter:** Standardmäßig wird nur `main.cpp` gebaut und der HAL-Code (`src/hal/`) ausgeblendet, da `SoftPWM` hier nicht einsetzbar ist.
* **Port (Beispiel):** `/dev/cu.usbmodemxxxx` (USB-C-Kabel)
* **Einsatz:** Experimente mit modernerem C++ und alternativer Hardware.

### Umgebung D: `env:xiao_esp32s3-debug` (XIAO ESP32S3 – Debug)

* **Basis:** Erweitert `env:xiao_esp32s3`
* **Zusatz:** `build_type = debug`, `-DDEBUG`, höheres `CORE_DEBUG_LEVEL`.
* **Zweck:** Analyse und Tests von Logik-Komponenten und neuen Features.

---

## 7. Workflow für Erweiterungen

Beispiel: Ein neuer Sensor (z. B. IR-Linienfolger) soll integriert werden.

1. **Hardware-Konfiguration**
   Pin in `include/Pins.h` als `constexpr` definieren, z. B.:

   ```cpp
   namespace Pins {
       constexpr uint8_t IR_LEFT  = A0;
       constexpr uint8_t IR_RIGHT = A1;
   }
   ```

2. **Treiber (HAL)**
   Klasse `IrSensor` in `src/hal/` implementieren:

   ```cpp
   class IrSensor {
   public:
       explicit IrSensor(uint8_t pin);
       int  readRaw() const;
       bool isLineDetected() const;
   };
   ```

3. **Logik (Behavior)**
   Auswertung der Sensordaten in `src/logic/` implementieren, z. B.:

   * Wenn $S_\text{left} < S_\text{threshold}$, Kurs nach links korrigieren.
   * Zustände `LineFound`, `LineLost` in der Bewegungslogik modellieren.

4. **Integration in die FSM**
   In `src/main.cpp` einen neuen Zustand `LineFollowing` ergänzen und Übergänge definieren:

   * `Idle → LineFollowing` (z. B. bei Tastendruck oder Start-Kommando)
   * `LineFollowing → Error` (z. B. wenn Sensorwerte ungültig sind)

5. **Tests**

   * Zunächst mit `env:uno-debug` testen (ausführliches Logging).
   * Danach sauberen Release-Build mit `env:uno`.

---

*Dokumentation aktualisiert am: 23.11.2025*
