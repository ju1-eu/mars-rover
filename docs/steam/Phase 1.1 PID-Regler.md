# Phase 1: Präzision & Orientierung (Math & Physics)

## 1.1 Vom P-Regler zum PID-Regler

Übergang von einem P-Regler zu einem **PID-Regler** ist der klassische Schritt vom "funktionierenden Modell" zur "industriellen Steuerung".

Hier ist die Implementierung der neuen Klasse `PidController`. Wir halten uns strikt an die **Schichten-Architektur**: Die Mathematik gehört in `src/logic/`, losgelöst von Hardware-Pins.

### 1. Die Mathematik (Theorie)
Bevor wir coden, hier die diskrete Formel, die wir implementieren:

$$
u(t) = \underbrace{K_p \cdot e(t)}_{\text{P: Gegenwart}} + \underbrace{K_i \cdot \sum e(t) \cdot \Delta t}_{\text{I: Vergangenheit}} + \underbrace{K_d \cdot \frac{e(t) - e(t-1)}{\Delta t}}_{\text{D: Zukunft}}
$$

* **P (Proportional):** Reagiert stark, wenn der Fehler groß ist.
* **I (Integral):** Summiert kleine Restfehler auf (z. B. wenn der Roboter auf Teppich dauerhaft leicht nach rechts zieht).
* **D (Derivative):** Bremst die Regelung ab, wenn wir uns dem Ziel schnell nähern (verhindert Überschwingen).

---

### 2. Header-Datei: `include/logic/PidController.h`
Wir erstellen eine saubere Klassendefinition.

```cpp
#pragma once
#include <Arduino.h> // Für abs(), constrain()

namespace Logic {

    class PidController {
    public:
        // Konstruktor mit den 3 Tuning-Parametern
        PidController(float kp, float ki, float kd);

        // Hauptmethode: Berechnet den Ausgabewert (z.B. Lenk-Korrektur)
        // setpoint: Wo wollen wir hin? (z.B. 0 Grad Gierrate)
        // measured: Wo sind wir? (z.B. 5 Grad Gierrate)
        // dt: Zeit seit dem letzten Aufruf in Sekunden
        float compute(float setpoint, float measured, float dt);

        // Methode zum Zurücksetzen (z.B. bei Neustart)
        void reset();

        // Setter für Runtime-Tuning (optional für spätere IoT-Phase)
        void setTunings(float kp, float ki, float kd);

    private:
        float _kp, _ki, _kd;
        float _prevError;
        float _integral;

        // Anti-Windup Limit (verhindert, dass I-Anteil ins Unendliche wächst)
        const float _integralLimit = 100.0f;
    };

}
```

---

### 3. Implementierung: `src/logic/PidController.cpp`
Hier passiert die eigentliche Arbeit. Beachten Sie den Schutz gegen "Integral Windup" (ein klassisches Ingenieurs-Problem, bei dem der I-Anteil zu groß wird, wenn der Motor blockiert ist).

```cpp
#include "logic/PidController.h"

namespace Logic {

    PidController::PidController(float kp, float ki, float kd)
        : _kp(kp), _ki(ki), _kd(kd), _prevError(0), _integral(0) {}

    float PidController::compute(float setpoint, float measured, float dt) {
        // 1. Fehler berechnen
        float error = setpoint - measured;

        // 2. Proportional-Anteil
        float P = _kp * error;

        // 3. Integral-Anteil (mit Zeitfaktor dt)
        _integral += error * dt;

        // Anti-Windup: Begrenzen des Integrals
        if (_integral > _integralLimit) _integral = _integralLimit;
        else if (_integral < -_integralLimit) _integral = -_integralLimit;

        float I = _ki * _integral;

        // 4. Derivative-Anteil (Steigung des Fehlers)
        // Schutz gegen Division durch Null
        float D = 0.0f;
        if (dt > 0.0001f) {
            D = _kd * ((error - _prevError) / dt);
        }

        // 5. Fehler für nächsten Durchlauf speichern
        _prevError = error;

        // 6. Summe zurückgeben
        return P + I + D;
    }

    void PidController::reset() {
        _prevError = 0;
        _integral = 0;
    }

    void PidController::setTunings(float kp, float ki, float kd) {
        _kp = kp;
        _ki = ki;
        _kd = kd;
    }
}
```

---

### 4. Integration: `src/logic/DriveAssistant.cpp`
Jetzt ersetzen wir den alten P-Regler im `DriveAssistant`.

```cpp
#include "logic/Motion.h"
#include "logic/PidController.h"
#include "hal/ImuSensor.h" // Angenommener Name für IMU Wrapper

// Instanz des PID-Reglers erstellen
// Startwerte (Tuning erforderlich!): Kp=2.0, Ki=0.5, Kd=0.1
static Logic::PidController yawPid(2.0f, 0.5f, 0.1f);

// Variablen für Zeitmessung (dt Berechnung)
static unsigned long lastTime = 0;

void DriveAssistant::update() {
    unsigned long now = millis();

    // dt in Sekunden berechnen (wichtig für PID Formel!)
    float dt = (now - lastTime) / 1000.0f;
    lastTime = now;

    // Sicherheits-Check: Wenn dt zu groß (z.B. erster Start), abbrechen
    if (dt > 0.1f || dt <= 0.0f) {
        dt = 0.0f; // Reset prevent jumps
    }

    // 1. Messen (HAL)
    float currentYawRate = Hal::Imu.getYawRate();

    // 2. Rechnen (Logic / Math)
    // Ziel ist 0 (geradeaus).
    float correction = yawPid.compute(0.0f, currentYawRate, dt);

    // 3. Anwenden (HAL / Motion)
    // Beispiel: Basisspeed 100. Korrektur wird addiert/subtrahiert.
    int leftSpeed = 100 - (int)correction;
    int rightSpeed = 100 + (int)correction;

    // Grenzen einhalten (0-255)
    leftSpeed = constrain(leftSpeed, -255, 255);
    rightSpeed = constrain(rightSpeed, -255, 255);

    Hal::Motors.setSpeed(leftSpeed, rightSpeed);
}
```

---

### 5. Tuning-Anleitung (Der STEAM-Labor-Teil)
Der Code allein reicht nicht. Die Schüler müssen die Werte $K_p, K_i, K_d$ finden. Geben Sie ihnen folgende Heuristik (Faustregel) an die Hand:

1.  **Alles auf 0 setzen.** ($K_p=0, K_i=0, K_d=0$). Der Rover fährt nicht geradeaus.
2.  **P erhöhen**, bis der Rover anfängt, schnell um die Linie zu **zappeln** (Oszillation).
3.  **P leicht reduzieren** (ca. 50-60% des Zappel-Werts). Jetzt fährt er stabil, aber vielleicht leicht schief bei Last (Steady-State Error).
4.  **I langsam erhöhen**, bis der Schiefstand verschwindet. Vorsicht: Zu viel I macht ihn träge und führt zu langsamem Schwingen.
5.  **D hinzufügen**, wenn der Rover beim abrupten Bremsen oder Anfahren zu stark nachwackelt. D wirkt wie ein virtueller Stoßdämpfer.

