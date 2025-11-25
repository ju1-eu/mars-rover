/**
 * @file       PidController.cpp
 * @brief      Implementierung des diskreten PID-Regelalgorithmus.
 * @author     Jan Unger
 * @version    1.1.0
 *
 * @details
 * Diese Klasse transformiert die physikalische Theorie der Regelungstechnik
 * in ausführbaren Code. Ein PID-Regler versucht, einen gemessenen Istwert
 * an einen gewünschten Sollwert anzunähern, indem er drei Strategien
 * kombiniert:
 *
 * 1. **P (Proportional):** "Gegenwart". Je größer der Fehler, desto stärker der
 * Gegendruck. Vergleichbar mit einer Feder.
 * 2. **I (Integral):** "Vergangenheit". Summiert Fehler über die Zeit auf.
 * Beseitigt bleibende Abweichungen (z.B. wenn der Rover auf schiefem Boden
 * fährt).
 * 3. **D (Derivative):** "Zukunft". Reagiert auf die Änderungsgeschwindigkeit.
 * Wirkt wie ein Stoßdämpfer, der ein Überschwingen verhindert.
 *
 * **Mathematisches Modell (Diskretisierung):**
 * Da der Mikrocontroller in Zeitschritten (Ticks) arbeitet, werden Integrale zu
 * Summen und Differentiale zu Differenzenquotienten:
 * @f$ u(t) = K_p \cdot e(t) + K_i \cdot \sum (e \cdot \Delta t) + K_d \cdot
 * \frac{e(t) - e(t-1)}{\Delta t} @f$
 */

#include "logic/PidController.h"
#include <math.h>

namespace Logic {

/**
 * @brief Konstruktor: Initialisiert die PID-Koeffizienten.
 * @param kp Verstärkungsfaktor P-Anteil.
 * @param ki Verstärkungsfaktor I-Anteil.
 * @param kd Verstärkungsfaktor D-Anteil.
 */
PidController::PidController(float kp, float ki, float kd)
    : _kp(kp), _ki(ki), _kd(kd), _prevError(0.0f), _integral(0.0f) {}

/**
 * @brief Hauptberechnungsschleife des Reglers.
 *
 * @param setpoint  Der gewünschte Zielwert (z.B. 0° Gierrate).
 * @param measured  Der aktuelle Sensorwert.
 * @param dt        Verstrichene Zeit seit dem letzten Aufruf in Sekunden.
 * Essenziell für korrekte Physik bei I- und D-Anteil!
 *
 * @return float    Die berechnete Stellgröße (u), die auf den Motor gegeben
 * wird.
 */
float PidController::compute(float setpoint, float measured, float dt) {
    // -----------------------------------------
    // 1. Regelabweichung bestimmen
    // -----------------------------------------
    // e(t) = w(t) - y(t)
    // Ein positiver Fehler bedeutet: "Wir sind noch unter dem Ziel".
    float error = setpoint - measured;

    // -----------------------------------------
    // 2. Proportional-Anteil (P)
    // "Die Kraft der Feder"
    // -----------------------------------------
    // Reagiert sofort auf den aktuellen Fehler.
    // Zu hoch -> System schwingt/zittert.
    // Zu niedrig -> System reagiert träge.
    float P = _kp * error;

    // -----------------------------------------
    // 3. Integral-Anteil (I)
    // "Das Gedächtnis"
    // -----------------------------------------
    // Euler-Integration (Rechteck-Regel): Fläche = Höhe * Breite
    // Wir addieren das Produkt aus Fehler und Zeit zum Speicher hinzu.
    _integral += error * dt;

    // @safety Anti-Windup (Clamping)
    // Problem: Wenn der Motor blockiert oder das Ziel unerreichbar ist,
    // wächst das Integral ins Unendliche ("Windup").
    // Folge: Sobald das Hindernis weg ist, würde der Rover noch sekundenlang
    // "falsch" fahren, um dieses riesige Integral wieder abzubauen.
    // Lösung: Wir deckeln den Speicher hart bei +/- _integralLimit.
    if (_integral > _integralLimit) {
        _integral = _integralLimit;
    } else if (_integral < -_integralLimit) {
        _integral = -_integralLimit;
    }

    float I = _ki * _integral;

    // -----------------------------------------
    // 4. Derivative-Anteil (D)
    // "Der Stoßdämpfer"
    // -----------------------------------------
    float D = 0.0f;

    // @safety Division-by-Zero Protection
    // Wenn dt extrem klein oder 0 ist (z.B. beim ersten Aufruf oder
    // Taktfehler), würde die Division zu Unendlich/NaN führen.
    if (dt > 0.0001f) {
        // Differenzenquotient: Steigung der Fehlerkurve.
        // (Aktueller Fehler - Letzter Fehler) / Zeit
        // Positiver Wert -> Fehler wird größer -> Dämpfung wirkt dagegen.
        float errorRate = (error - _prevError) / dt;
        D = _kd * errorRate;
    }

    // -----------------------------------------
    // 5. Housekeeping
    // -----------------------------------------
    // Den aktuellen Fehler für den nächsten Zyklus speichern (als e(t-1)).
    _prevError = error;

    // -----------------------------------------
    // 6. Superposition (Ausgang)
    // -----------------------------------------
    // Die Summe aller drei Anteile ergibt die Stellgröße u(t).
    return P + I + D;
}

/**
 * @brief Setzt den internen Speicher des Reglers zurück.
 *
 * @details
 * Löscht den I-Speicher und den Fehler-Verlauf.
 * **Wann nutzen?**
 * Immer dann, wenn sich der Betriebsmodus ändert (z.B. von "Wende" zu
 * "Geradeaus"). Ohne Reset würde der Regler versuchen, Fehler aus der Wende
 * noch während der Geradeausfahrt zu korrigieren ("Geister-Drift").
 */
void PidController::reset() {
    _prevError = 0.0f;
    _integral = 0.0f;
}

/**
 * @brief Ermöglicht das Anpassen der Parameter zur Laufzeit.
 *
 * @note Nützlich für Remote-Tuning via Bluetooth/WiFi, ohne neu zu kompilieren.
 */
void PidController::setTunings(float kp, float ki, float kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

} // namespace Logic
