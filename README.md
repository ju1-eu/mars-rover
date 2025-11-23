# \# GalaxyRVR Profi - Firmware Dokumentation

## 1\. Projektübersicht

Dies ist eine professionell strukturierte Firmware-Entwicklungsumgebung für zwei parallele Plattformen. Das Projekt verzichtet auf monolithischen "Spaghetti-Code" und implementiert stattdessen eine **Schichten-Architektur (Layered Architecture)**, moderne C++ Standards und **Non-Blocking I/O**.

### Ziel-Plattformen

1.  **SunFounder Galaxy RVR**
      * **Hardware:** Arduino Uno R3 (ATmega328P) + Galaxy RVR Shield
      * **Fokus:** Mobile Robotik, Motorsteuerung, Sensorik
      * **Standard:** C++17 (via `avr-g++`)
2.  **Seeed Studio XIAO ESP32S3**
      * **Hardware:** ESP32-S3 (Dual-Core Xtensa LX7)
      * **Fokus:** Experimentierplattform für High-Level C++ Features
      * **Standard:** C++20

-----

## 2\. Architektur-Konzept

Das System folgt dem Prinzip der **Separation of Concerns**. Die Firmware ist in drei horizontale Schichten unterteilt.

### Schicht 1: HAL (Hardware Abstraction Layer)

  * **Verzeichnis:** `src/hal/`
  * **Aufgabe:** Physischer Zugriff auf die Hardware (Pins, PWM-Register, ADC).
  * **Implementierung:** Nutzt plattformspezifische Frameworks (z. B. `SoftPWM` für AVR).
  * **Schnittstelle:** Bietet abstrakte Methoden wie `setSpeed(int, int)` an.

### Schicht 2: Logik & Verhalten (Logic Layer)

  * **Verzeichnis:** `src/logic/`
  * **Aufgabe:** Berechnung von Bewegungsvektoren und Regelkreisen.
  * **Mathematik:** Hier werden abstrakte Befehle in konkrete Motorwerte umgerechnet.
      * *Beispiel Kurvenfahrt:*
        Bei einer Geschwindigkeit $v \in [0, 255]$ und einem Kurvenverhältnis $r \in [0.0, 1.0]$ berechnet sich die Geschwindigkeit des kurveninneren Rades $v_\text{in}$ wie folgt:
        $$v_\text{in} = v \cdot (1.0 - r)$$
  * **Abhängigkeit:** Kennt nur die HAL, keine Pin-Nummern.

### Schicht 3: Anwendung (Application Layer)

  * **Verzeichnis:** `src/main.cpp`
  * **Aufgabe:** High-Level Steuerung durch einen **Endlichen Automaten (FSM)**.
  * **Timing:** Nutzt `millis()` für nicht-blockierende Zeitsteuerung gemäß:
    $$t_\text{now} - t_\text{last} > \Delta t$$

-----

## 3\. Ordnerstruktur

Die Struktur ist für **PlatformIO** optimiert und trennt öffentliche Schnittstellen (`include`), private Implementierungen (`src`) und Dokumentation (`docs`).

```text
GalaxyRVR_Profi/
├── docs/
│   ├── archive/            # Alter Code (z.B. Lektion04_MotorTest.cpp)
│   ├── datasheets/         # Hardware-Datenblätter & Schaltpläne
│   └── learning_notes/     # Lern-Notizen & Konzepte (.md)
├── include/                # Globale Header (Konfiguration & Schnittstellen)
│   ├── Config.h            # Systemweite Konstanten (constexpr)
│   ├── Motion.h            # Schnittstelle für Bewegungslogik
│   └── Pins.h              # Pin-Mapping für den Galaxy RVR
├── src/                    # Quellcode
│   ├── hal/                # Hardware-Treiber (z.B. Motor.cpp)
│   ├── logic/              # Plattformunabhängige Algorithmen
│   └── main.cpp            # Einstiegspunkt & State Machine
├── test/                   # Experimenteller Code (z.B. ESP32 Tests)
└── platformio.ini          # Build-Konfiguration & Environments
```

-----

## 4\. Konfiguration & Hardware (Galaxy RVR)

### Pin-Mapping

Das Pinning ist in `include/Pins.h` als `constexpr` definiert. Aufgrund der Hardware-Limitierung des ATmega328P (nur 6 Hardware-PWM Kanäle) wird für spezifische Pins eine Software-PWM Lösung benötigt.

| Funktion | Arduino Pin | Typ | Besonderheit |
| :--- | :--- | :--- | :--- |
| **Motor Links** (Fwd) | `2` | Digital | Benötigt `SoftPWM` |
| **Motor Links** (Rev) | `3` | PWM | Hardware PWM |
| **Motor Rechts** (Rev)| `4` | Digital | Benötigt `SoftPWM` |
| **Motor Rechts** (Fwd)| `5` | PWM | Hardware PWM |

### Mathematische Modelle

In `src/logic/Motion.cpp` wird die differentielle Lenkung berechnet. Die PWM-Werte $PWM_L$ und $PWM_R$ ergeben sich aus der Soll-Geschwindigkeit $v$ und dem Lenk-Faktor.

Für eine Drehung auf der Stelle ("Panzer-Wende") gilt:
$$\text{PWM_L} = -v, \quad \text{PWM_R} = +v$$

-----

## 5\. Coding Guidelines

Wir folgen strikten Richtlinien für **Embedded C++**:

1.  **Non-Blocking I/O:**
    Verzicht auf `delay()`. Alle zeitkritischen Aufgaben werden über Polling der Systemzeit gesteuert.
2.  **Starke Typisierung:**
    Verwendung von `enum class` für Zustände, um implizite Casts zu `int` zu verhindern.
3.  **Compiler-Standards:**
      * **AVR:** `-std=gnu++17` (C++17 mit GNU Extensions)
      * **ESP32:** `-std=gnu++2a` (C++20 Support, z. B. für `std::vector`, Designated Initializers)
4.  **Ressourcen-Management:**
    Nutzung von `constexpr` für Konstanten, um RAM-Verbrauch zur Laufzeit zu minimieren.

-----

## 6\. Build-Umgebungen (Environments)

Das Projekt wird über `platformio.ini` gesteuert und enthält zwei getrennte Umgebungen.

### Umgebung A: `env:uno` (Galaxy RVR)

  * **Plattform:** Atmel AVR
  * **Board:** Arduino Uno
  * **Code:** Kompiliert `src/hal/` und nutzt `Bhagman SoftPWM`.
  * **Port:** `/dev/cu.usbserial-xxxx` (USB-B Kabel)

### Umgebung B: `env:xiao_esp32s3` (Experimentell)

  * **Plattform:** Espressif 32
  * **Board:** Seeed Studio XIAO ESP32S3
  * **Filter:** Ignoriert `src/hal/` (via `build_src_filter`), da SoftPWM nicht kompatibel ist.
  * **Port:** `/dev/cu.usbmodemxxxx` (USB-C Kabel)
  * **Features:** Nutzt C++20 Features (teilweise eingeschränkt durch GCC 8.4).

-----

## 7\. Workflow für Erweiterungen

Um einen neuen Sensor (z. B. IR-Linienfolger) hinzuzufügen:

1.  **Hardware:** Pin in `include/Pins.h` definieren.
2.  **Treiber:** Klasse `IrSensor` in `src/hal/` erstellen.
3.  **Logik:** Auswertung der Sensordaten (z. B. $S_\text{left} < S_\text{threshold}$) in `src/logic/` implementieren.
4.  **Integration:** Zustand `LineFollowing` in der FSM in `src/main.cpp` ergänzen.

-----

*Dokumentation aktualisiert am: 22.11.2025*
