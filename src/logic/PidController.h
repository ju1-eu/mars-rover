/**
 * @file       PidController.h
 * @brief      Generische PID-Regler-Klasse.
 *
 * @details
 * Diese Klasse implementiert einen zeitdiskreten PID-Regler
 * (Proportional-Integral-Derivative). Er wird verwendet, um physikalische
 * Größen (wie die Gierrate des Rovers) auf einen gewünschten Sollwert zu
 * regeln.
 *
 * Theorie:
 * - P-Anteil: Reagiert auf den aktuellen Fehler (Gegenwart).
 * - I-Anteil: Reagiert auf aufsummierte Fehler (Vergangenheit / stationäre
 * Genauigkeit).
 * - D-Anteil: Reagiert auf die Änderungsrate des Fehlers (Zukunft / Dämpfung).
 */

#pragma once

#include <Arduino.h> // Nötig für Datentypen und mathematische Hilfsfunktionen

namespace Logic {

class PidController {
  public:
    /**
     * @brief Konstruktor: Initialisiert den Regler mit Tuning-Parametern.
     *
     * @param kp Proportional-Beiwert (Reaktionsstärke)
     * @param ki Integral-Beiwert (Fehlerbeseitigung über Zeit)
     * @param kd Derivative-Beiwert (Dämpfung von Schwingungen)
     */
    PidController(float kp, float ki, float kd);

    /**
     * @brief Berechnet den Stellwert (Output) für den aktuellen Zeitschritt.
     *
     * Dies ist die Kern-Funktion, die zyklisch (z. B. alle 50ms) aufgerufen
     * werden muss.
     *
     * @param setpoint  Der Zielwert (Sollgröße, z.B. 0.0° Gierrate für
     * Geradeaus).
     * @param measured  Der aktuelle Istwert (Messgröße vom Sensor).
     * @param dt        Vergangene Zeit seit dem letzten Aufruf in Sekunden.
     * Wichtig: Muss präzise sein, damit I- und D-Anteil korrekt skalieren!
     *
     * @return          Der berechnete Korrekturwert (Stellgröße u).
     */
    float compute(float setpoint, float measured, float dt);

    /**
     * @brief Setzt den internen Speicher des Reglers zurück.
     *
     * Löscht den aufsummierten Integral-Fehler und den "letzten Fehler".
     * Wichtig beim Start einer neuen Mission oder nach einem Not-Stopp,
     * um unvorhersehbares Verhalten ("Losrasen") zu verhindern.
     */
    void reset();

    /**
     * @brief Ermöglicht das Anpassen der Parameter zur Laufzeit.
     *
     * Nützlich für spätere Phasen (IoT), um Tuning via Web-Interface
     * vorzunehmen, ohne den Code neu zu kompilieren.
     *
     * @param kp Neuer Proportional-Wert
     * @param ki Neuer Integral-Wert
     * @param kd Neuer Derivative-Wert
     */
    void setTunings(float kp, float ki, float kd);

  private:
    // Tuning-Parameter
    float _kp;
    float _ki;
    float _kd;

    // Speicher-Variablen für Zeit-Operationen
    float
        _prevError;  // Fehler des vorherigen Schritts (für D-Anteil / Steigung)
    float _integral; // Aufsummierter Fehler (für I-Anteil / Fläche)

    /**
     * @brief Anti-Windup Limit.
     *
     * Begrenzt den Integral-Anteil auf +/- 100.0.
     * Verhindert, dass der I-Anteil bei blockiertem System (z. B. Rover fährt
     * gegen Wand) ins Unendliche wächst ("Windup"). Ohne dies würde der Rover
     * nach Befreiung lange Zeit rückwärts fahren, um das "überschüssige
     * Integral abzubauen".
     */
    const float _integralLimit = 100.0f;
};

} // namespace Logic
