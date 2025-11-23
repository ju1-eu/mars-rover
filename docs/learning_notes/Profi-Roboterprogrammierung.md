# Profi-Roboterprogrammierung: C++17 Clean Code Konzept

Wer "Profi-Roboterprogrammierung" lernen will, muss sich von zwei schlechten Angewohnheiten verabschieden:

1.  **`delay()`**: Weil der Roboter während des Wartens "blind" und "taub" ist.
2.  **Magische Zahlen**: Weil niemand nach zwei Wochen noch weiß, was `digitalWrite(5, 1)` bedeutet.

Dieses Konzept basiert auf drei Säulen, die den Galaxy RVR reaktiv, wartbar und skalierbar machen: **Prozedurale Struktur**, **State Machines** (Zustandsautomaten) und **nicht-blockierendes Timing**.

## Die 3 Säulen des Konzepts

1.  **Strong Typing (`enum class`):**
    Wir nutzen keine primitiven Datentypen wie `int` für Zustände (0, 1, 2), sondern semantisch klare Typen.
2.  **Non-Blocking (kein `delay`):**
    Wir nutzen Zeitstempel (`millis()`), damit der Roboter *gleichzeitig* Sensoren auswerten und Aktoren steuern kann.
3.  **HAL (Hardware Abstraction Layer):**
    Die Hauptschleife (`loop()`) darf nicht wissen, an welchem Pin der Motor hängt. Sie ruft abstrahierte Befehle wie `Motor::fahren()` auf.

-----

## Der Code: Autonomer Rover mit State Machine

Kopiere den folgenden Code in deine `src/main.cpp` (z. B. in VS Code / PlatformIO).

```cpp
#include <Arduino.h>

// ============================================================
// 1. KONFIGURATION (Compile-Time Constants)
// ============================================================
namespace Pin {
    constexpr uint8_t MotorLinks       = 5;
    constexpr uint8_t MotorRechts      = 6;
    constexpr uint8_t UltraschallTrigger = 12;
    constexpr uint8_t UltraschallEcho    = 13;
}

namespace Config {
    constexpr unsigned long SensorIntervallMs = 100; // Sensor nur alle 100ms lesen
    constexpr int HindernisDistanzCm      = 25;
    constexpr uint8_t SpeedNormal         = 120;
    constexpr uint8_t SpeedDrehen         = 140;
}

// ============================================================
// 2. TYPEN & ZUSTÄNDE (Strong Typing C++17)
// ============================================================

// Definiert die möglichen Verhaltensweisen des Roboters
enum class RoboterZustand {
    Erkunden,   // Geradeaus fahren
    Ausweichen, // Hindernis erkannt, drehen
    Stopp       // Fehler oder Pause
};

// ============================================================
// 3. GLOBALE VARIABLEN (Minimal halten!)
// ============================================================
RoboterZustand aktuellerZustand = RoboterZustand::Stopp;
unsigned long letzterSensorCheck = 0;
int aktuelleDistanz = 0;

// ============================================================
// 4. HARDWARE ABSTRACTION LAYER (HAL)
// Hier kapseln wir die "schmutzigen" Arduino-Befehle
// ============================================================

namespace Motor {
    void init() {
        pinMode(Pin::MotorLinks, OUTPUT);
        pinMode(Pin::MotorRechts, OUTPUT);
    }

    void halt() {
        analogWrite(Pin::MotorLinks, 0);
        analogWrite(Pin::MotorRechts, 0);
    }

    void vorwaerts(uint8_t speed) {
        // Galaxy RVR Logik: Beide Motoren an
        analogWrite(Pin::MotorLinks, speed);
        analogWrite(Pin::MotorRechts, speed);
    }

    void drehen(uint8_t speed) {
        // Simples Drehen: Ein Motor aus, einer an (oder gegenläufig)
        analogWrite(Pin::MotorLinks, 0);
        analogWrite(Pin::MotorRechts, speed);
    }
}

namespace Sensor {
    void init() {
        pinMode(Pin::UltraschallTrigger, OUTPUT);
        pinMode(Pin::UltraschallEcho, INPUT);
    }

    // [[nodiscard]] warnt, wenn das Ergebnis ignoriert wird
    [[nodiscard]] int messeDistanzCm() {
        digitalWrite(Pin::UltraschallTrigger, LOW);
        delayMicroseconds(2);
        digitalWrite(Pin::UltraschallTrigger, HIGH);
        delayMicroseconds(10);
        digitalWrite(Pin::UltraschallTrigger, LOW);

        auto dauer = pulseIn(Pin::UltraschallEcho, HIGH, 25000); // Timeout 25ms
        if (dauer == 0) return 999; // Kein Echo = Weg frei

        // Berechnung: Weg = Zeit * Schallgeschwindigkeit / 2
        return static_cast<int>(dauer * 0.034 / 2);
    }
}

// ============================================================
// 5. LOGIK & STEUERUNG
// ============================================================

void bearbeiteZustand() {
    switch (aktuellerZustand) {
        case RoboterZustand::Erkunden:
            Motor::vorwaerts(Config::SpeedNormal);

            // Zustandsübergang (Transition)
            if (aktuelleDistanz < Config::HindernisDistanzCm) {
                // Sofort stoppen bevor wir den Zustand wechseln
                Motor::halt();
                aktuellerZustand = RoboterZustand::Ausweichen;
            }
            break;

        case RoboterZustand::Ausweichen:
            Motor::drehen(Config::SpeedDrehen);

            // Zustandsübergang mit Hysterese
            if (aktuelleDistanz > (Config::HindernisDistanzCm + 5)) {
                // +5cm Hysterese verhindert Zittern
                Motor::halt();
                aktuellerZustand = RoboterZustand::Erkunden;
            }
            break;

        case RoboterZustand::Stopp:
            Motor::halt();
            break;
    }
}

// ============================================================
// 6. ARDUINO STANDARD FUNKTIONEN
// ============================================================

void setup() {
    Serial.begin(9600);
    Motor::init();
    Sensor::init();

    // Startzustand setzen
    aktuellerZustand = RoboterZustand::Erkunden;
    Serial.println("System bereit. Starte Erkundung.");
}

void loop() {
    // --- SCHRITT 1: EINGABE (Non-Blocking) ---
    unsigned long jetzt = millis();

    if (jetzt - letzterSensorCheck >= Config::SensorIntervallMs) {
        aktuelleDistanz = Sensor::messeDistanzCm();
        letzterSensorCheck = jetzt;
    }

    // --- SCHRITT 2: VERARBEITUNG (State Machine) ---
    bearbeiteZustand();

    // --- SCHRITT 3: AUSGABE ---
    // (Erfolgt gekapselt in den Motor-Funktionen)
}
```

-----

## Analyse: Was macht diesen Code "Profi"?

### 1\. Die State Machine (Zustandsautomat)

Anfänger schreiben verschachtelte `if`-Anweisungen mit `delay()`, was den Prozessor blockiert.
Profis definieren eine **Finite State Machine (FSM)**. In jedem Durchlauf der `loop` (die tausende Male pro Sekunde läuft) wird geprüft:

  * *In welchem Zustand bin ich?*
  * *Welche Aktion ist hier nötig?*
  * *Ist eine Bedingung für den Zustandswechsel erfüllt?*

Dadurch reagiert der Roboter **sofort** auf Änderungen.

### 2\. Non-Blocking Timer (`millis`)

Wir nutzen die Differenz der Systemzeit:
$$t_{delta} = t_{jetzt} - t_{letzterCheck}$$
Nur wenn $t_{delta} \ge \text{Intervall}$ ist, wird gemessen.
Das bedeutet: Der Ultraschall wird nur z. B. 10x pro Sekunde gefeuert. In der restlichen Zeit (99% der CPU-Zeit) kann der Prozessor andere Aufgaben erledigen (LEDs blinken, WiFi senden).

### 3\. Namensräume (`namespace`)

Durch `namespace Motor` und `namespace Sensor` trennst du die **Hardware-Details** von der **Logik**.

  * `Motor::vorwaerts()` ist selbsterklärend.
  * Wenn sich die Hardware ändert (z. B. anderer Treiber), muss nur der Namespace angepasst werden, nicht die gesamte Logik.

### 4\. Hysterese (Schwellenwert-Puffer)

Im Code findest du:

```cpp
if (aktuelleDistanz > (Config::HindernisDistanzCm + 5))
```

Das verhindert das "Zittern" an der Grenzlinie.

  * Stopp bei $d < 25\text{cm}$.
  * Weiterfahrt erst bei $d > 30\text{cm}$.
    Dieser Bereich von $5\text{cm}$ nennt sich Hysterese und sorgt für ein stabiles Verhalten.

-----

## Prinzipien der Embedded-Softwareentwicklung

| Prinzip | Erklärung | Vorteil |
| :--- | :--- | :--- |
| **Architektur & HAL** | Kapselung von Hardware-Befehlen in Namespaces oder Klassen. | Code ist lesbar; Hardware-Tausch erfordert kein Rewrite der Logik. |
| **Zeitmanagement** | Verzicht auf `delay()`; Nutzung von Polling mit `millis()`. | Multitasking-Fähigkeit; Roboter bleibt jederzeit reaktionsfähig. |
| **Zustandsautomat** | Nutzung von `enum class` und `switch-case` zur Steuerung. | Verhindert "Spaghetti-Code"; komplexe Abläufe werden planbar. |
| **Clean Code** | `constexpr` statt "Magic Numbers"; sprechende Variablennamen. | Wartbarkeit und Typsicherheit (Compile-Time Checks). |


---

## Anleitung für Kommentare auf Profi-Niveau

### 1\. Die Kernprinzipien (Das "Warum", nicht das "Was")

In professionellen Umgebungen (Automotive, Medizintechnik, Intralogistik) gelten folgende Grundregeln:

  * **Intention vor Implementierung:** Erklären Sie, *warum* sich der Roboter bewegt, nicht *dass* er sich bewegt.
      * *Schlecht:* `PTP HOME ; Fahre zur Home-Position` (Redundant).
      * *Gut:* `PTP HOME ; Rückzug zur sicheren Wartungsposition, um Kollision mit Drehtisch zu vermeiden.`
  * **Physikalischer Kontext:** Kommentare müssen Hardware-Abhängigkeiten beschreiben.
      * Beispiel: `; Wartezeit 0.5s notwendig für Druckaufbau im Greifer (siehe Pneumatik-Plan S. 12).`
  * **Sprache:** Englisch ist der Industriestandard, auch in deutschen Firmen, um internationale Wartungsteams zu unterstützen. Bitte trotzdem deutsch.

-----

### 2\. High-Level Robotik (C++, Python, ROS/ROS2)

Hier gelten Software-Engineering-Standards. Nutzen Sie **Dokumentations-Generatoren**.

  * **Doxygen (C++):** Standard für professionelle Dokumentation. Ermöglicht das automatische Erstellen von Handbüchern aus dem Code.
  * **Docstrings (Python):** Nutzen Sie PEP 257.

**Beispiel (C++ / ROS):**

```cpp
/**
 * @brief Calculates the inverse kinematics for the 6-DOF arm.
 * * This function uses the analytical method described in Paper X.
 * WARNING: Returns error if target is arguably close to singularity.
 * * @param target_pose The desired end-effector position (geometry_msgs/Pose).
 * @return JointState The calculated joint angles or null on failure.
 */
JointState calculateIK(const Pose& target_pose) { ... }
```

-----

### 3\. Kritische Best Practices & Warnhinweise

#### TODOs und Fixmes

Nutzen Sie Tags, die von IDEs (VS Code, Visual Studio) erkannt werden, aber halten Sie diese aktuell.

  * `// TODO: Implement collision check with camera data.`
  * `// FIXME: Temporary workaround for sensor jitter. Remove after firmware update v2.0.`

#### Magic Numbers vermeiden

Kommentieren Sie niemals einfach nur eine nackte Zahl. Besser: Ersetzen Sie die Zahl durch eine Konstante. Wenn eine Zahl stehen bleiben muss, erklären Sie ihre Herkunft.

  * *Schlecht:* `WaitTime(0.2);`
  * *Profi:* `WaitTime(0.2); // 200ms delay required for PLC handshake signal stability`

#### Referenzierung auf externe Dokumente

In der Industrie ist der Code Teil eines Systems. Verlinken Sie auf Tickets oder Pläne.

  * `// Logic changed per Change Request CR-1024 (Optimierung Taktzeit).`
  * `// E/A-Belegung entspricht Schaltplan EPLAN Seite 42/Spalte 3.`

-----

### 4\. Vergleich: Amateur vs. Profi

| Merkmal | Amateur / Hobby | Profi / Industrie |
| :--- | :--- | :--- |
| **Inhalt** | Beschreibt, was der Code tut. | Beschreibt die *Intention* und *Nebenbedingungen*. |
| **Wartung** | Keine Versionshistorie. | Header mit Datum, Autor und Änderungsgrund. |
| **Warnungen** | Keine. | Explizite Warnungen vor physischen Gefahren (Kollision, Quetschgefahr). |
| **Einheiten** | Oft unklar. | Einheiten explizit genannt (`// Speed in m/s`). |

-----

### 5\. Zusammenfassung der Strategie

1.  **Header:** Beginnen Sie jede Datei mit Metadaten (Autor, Zweck, Datum, Abhängigkeiten).
2.  **API-Doku:** Nutzen Sie Doxygen/Docstrings für Funktionen, die von anderen aufgerufen werden.
3.  **Inline:** Erklären Sie komplexe Logik oder "Hacks", die nötig waren, um Hardware-Eigenheiten zu umgehen.
4.  **Aufräumen:** Auskommentierter Code ("Zombie Code") muss vor dem Deployment gelöscht werden. Versionskontrolle (Git) speichert die Geschichte, nicht die Datei selbst.
