/**
 * @file       MotionSequence.cpp
 * @brief      Sequenzsteuerung (State Machine) für autonome Robotermanöver.
 * @details    Implementiert eine nicht-blockierende Zustandsmaschine zur
 * Steuerung der Antriebslogik. Nutzt eine HAL-Abstraktion (Logic::Motion).
 * * Ablauf: Vorwärts -> Rückwärts -> Links -> Rechts -> Stop -> Loop
 * @author     Jan Unger
 * @version    1.0.0
 * @date       2025-11-22
 * * @warning    OPEN LOOP STEUERUNG: Die Bewegung erfolgt zeitgesteuert ohne
 * Encoder-Feedback. Positioniergenauigkeit variiert mit Batteriespannung.
 * @safety     Sicherstellen, dass der Arbeitsbereich (Radius 0.5m) frei ist.
 */

#include "Motion.h" // Hardware Abstraction Layer (HAL) für Motortreiber
#include <Arduino.h>

// --- Konfiguration & Konstanten ---

/** * @brief PWM-Wert für die Motorgeschwindigkeit.
 * @note  Bereich: 0-255. Wert < 150 kann zu Stalling (Stillstand unter Last)
 * führen.
 */
constexpr uint8_t kSpeed = 200;

/** * @brief Zeitdauer für aktive Bewegungsphasen.
 * @unit  Millisekunden (ms)
 */
constexpr unsigned long kDurationMove = 1000;

/** * @brief Wartezeit im Endzustand vor Neustart der Sequenz.
 * @unit  Millisekunden (ms)
 */
constexpr unsigned long kDurationStop = 5000;

/** * @brief Pausenzeit zwischen Richtungswechseln zur Schonung der Mechanik
 * (Ruckbegrenzung).
 * @unit  Millisekunden (ms)
 */
constexpr unsigned long kPauseDuration = 1000;

/**
 * @enum  State
 * @brief Zustände der Motion-Control State Machine.
 */
enum class State {
    Start,    ///< Initialisierung und erster Bewegungsbefehl
    Forward,  ///< Roboter fährt geradeaus
    Pause1,   ///< Stillstand zum Abbau kinetischer Energie
    Backward, ///< Roboter fährt rückwärts
    Pause2,   ///< Stillstand
    Left,     ///< Punkt-Drehung (Pivot) nach links
    Pause3,   ///< Stillstand
    Right,    ///< Punkt-Drehung (Pivot) nach rechts
    Finished  ///< Zyklus beendet, Wartezeit vor Neustart
};

// Globale Status-Variablen
State currentState = State::Start;
unsigned long lastStateChange = 0;

/**
 * @brief Initialisiert serielle Kommunikation und Motor-Hardware.
 */
void setup() {
    Serial.begin(115200);
    Serial.println("--- System Boot: Motion Control Lektion 5 ---");

    // Initialisierung der Hardware-Schicht (Treiber-Pins, PWM-Timer)
    Logic::Motion::init();

    // BLOCKING: Sicherheitsverzögerung (2s)
    // Ermöglicht dem Bediener, sich nach dem Einschalten zu entfernen.
    delay(2000);
}

/**
 * @brief Hauptschleife (Zykluszeit typ. < 1ms).
 * Führt die Zustandsmaschine aus.
 */
void loop() {
    // Erfassung der aktuellen Systemzeit für nicht-blockierendes Timing
    unsigned long now = millis();
    unsigned long timeInState = now - lastStateChange;

    switch (currentState) {
    // ---------------------------------------------------------
    // PHASE 1: Vorwärtsbewegung
    // ---------------------------------------------------------
    case State::Start:
        Serial.println("[STATE] Start -> Forward");
        Logic::Motion::moveForward(kSpeed); // Aktorik aktivieren

        // Zustandsübergang
        lastStateChange = now;
        currentState = State::Forward;
        break;

    case State::Forward:
        // Bedingung: Bewegungsdauer abgelaufen
        if (timeInState > kDurationMove) {
            Serial.println("[STATE] Forward -> Pause");
            Logic::Motion::stopMove(); // Wichtig: Aktoren explizit stoppen

            lastStateChange = now;
            currentState = State::Pause1;
        }
        break;

    // ---------------------------------------------------------
    // PHASE 2: Rückwärtsbewegung
    // ---------------------------------------------------------
    case State::Pause1:
        // Nutzung der Konstante kPauseDuration statt "Magic Number" 1000
        if (timeInState > kPauseDuration) {
            Serial.println("[STATE] Pause -> Backward");
            Logic::Motion::moveBackward(kSpeed);

            lastStateChange = now;
            currentState = State::Backward;
        }
        break;

    case State::Backward:
        if (timeInState > kDurationMove) {
            Serial.println("[STATE] Backward -> Pause");
            Logic::Motion::stopMove();

            lastStateChange = now;
            currentState = State::Pause2;
        }
        break;

    // ---------------------------------------------------------
    // PHASE 3: Rotation (Pivot Turns)
    // ---------------------------------------------------------
    case State::Pause2:
        if (timeInState > kPauseDuration) {
            Serial.println("[STATE] Pause -> Turn Left");
            // Differenzialantrieb: Linke Seite zurück, rechte Seite vor
            Logic::Motion::turnLeft(kSpeed);

            lastStateChange = now;
            currentState = State::Left;
        }
        break;

    case State::Left:
        if (timeInState > kDurationMove) {
            Serial.println("[STATE] Turn Left -> Pause");
            Logic::Motion::stopMove();

            lastStateChange = now;
            currentState = State::Pause3;
        }
        break;

    case State::Pause3:
        if (timeInState > kPauseDuration) {
            Serial.println("[STATE] Pause -> Turn Right");
            Logic::Motion::turnRight(kSpeed);

            lastStateChange = now;
            currentState = State::Right;
        }
        break;

    case State::Right:
        if (timeInState > kDurationMove) {
            Serial.println("[STATE] Sequence Complete -> Standby");
            Logic::Motion::stopMove(); // Sicherer Zustand

            lastStateChange = now;
            currentState = State::Finished;
        }
        break;

    // ---------------------------------------------------------
    // PHASE 4: Zyklus-Ende
    // ---------------------------------------------------------
    case State::Finished:
        // Wartet kDurationStop ab, bevor die State Machine zurückgesetzt wird.
        if (timeInState > kDurationStop) {
            Serial.println("[RESET] Restarting Sequence");
            currentState = State::Start; // Loop-Restart
        }
        break;
    }
}
