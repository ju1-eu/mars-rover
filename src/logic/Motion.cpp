/**
 * @file       Motion.cpp
 * @brief      Implementierung der Antriebslogik (Kinematik-Layer).
 * @details    Übersetzt High-Level Fahrbefehle (Vorwärts, Drehen) in
 * spezifische Motoransteuerungen für einen Differenzialantrieb (Differential
 * Drive). Dient als Abstraktionsschicht zwischen Applikation und HAL.
 * @author     Jan Unger
 * @version    1.0.0
 * @date       2025-11-22
 */

#include "Motion.h"
#include "../hal/Motor.h" // Direkte Abhängigkeit zum Hardware-Treiber

namespace Logic::Motion {

/**
 * @brief Initialisiert die unterlagerte Motor-Hardware.
 * Leitet den Aufruf direkt an den HAL-Treiber weiter.
 */
void init() { HAL::Motor::init(); }

/**
 * @brief Fährt geradeaus vorwärts.
 * @param speed PWM-Geschwindigkeit (0-255).
 * Wird auf beide Motoren positiv angewendet.
 */
void moveForward(uint8_t speed) {
    // Mapping: Beide Motoren gleiche Polarität (+)
    HAL::Motor::setSpeed(speed, speed);
}

/**
 * @brief Fährt geradeaus rückwärts.
 * @param speed PWM-Geschwindigkeit (0-255).
 * @note  Der uint8_t Wert wird hier negiert. Der HAL-Treiber muss
 * signed integer (int16_t/int) akzeptieren, um die Drehrichtung umzukehren.
 */
void moveBackward(uint8_t speed) {
    // Mapping: Beide Motoren invertierte Polarität (-)
    HAL::Motor::setSpeed(-speed, -speed);
}

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot Turn) nach links aus.
 * @details Die Räder drehen gegenläufig, um den Wenderadius auf 0 zu setzen.
 * Dies erfordert hohe Traktion und Drehmoment.
 * @param speed Geschwindigkeit der Rotation.
 */
void turnLeft(uint8_t speed) {
    // Kinematik: Links rückwärts (-), Rechts vorwärts (+)
    // Resultat: Rotation gegen den Uhrzeigersinn (CCW)
    HAL::Motor::setSpeed(-speed, speed);
}

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot Turn) nach rechts aus.
 * @param speed Geschwindigkeit der Rotation.
 */
void turnRight(uint8_t speed) {
    // Kinematik: Links vorwärts (+), Rechts rückwärts (-)
    // Resultat: Rotation im Uhrzeigersinn (CW)
    HAL::Motor::setSpeed(speed, -speed);
}

/**
 * @brief Stoppt alle Antriebe sofort (Hard Stop).
 * @warning Abhängig von der HAL-Implementierung kann dies
 * Segeln (Coasting) oder aktives Bremsen (Braking) bedeuten.
 */
void stopMove() { HAL::Motor::stop(); }

} // namespace Logic::Motion
