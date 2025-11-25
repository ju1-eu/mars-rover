/**
 * @file       Motion.cpp
 * @brief      Basis-Kinematik für Differentialantrieb (Open Loop).
 *
 * @details
 * Dieses Modul implementiert die grundlegenden Bewegungsmuster ("Primitives")
 * für einen Roboter mit Differentialantrieb.
 *
 * Architektur-Einordnung:
 * - Es handelt sich um eine **Open-Loop-Steuerung** (Steuerung).
 * - Es findet KEINE Rückkopplung durch Sensoren statt.
 * - Befehle wie "Fahre geradeaus" gehen davon aus, dass beide Motoren
 * exakt gleich schnell drehen (was physikalisch selten der Fall ist).
 *
 * Für präzise, geregelte Fahrten (Closed Loop) sollte stattdessen der
 * @c Logic::DriveAssistant genutzt werden.
 *
 * @dependency HAL::Motor
 */

#include "logic/Motion.h"
#include "hal/Motor.h" // Zugriff auf Hardware-Treiber

namespace Logic::Motion {

/**
 * @brief Initialisierung der Bewegungsschicht.
 */
void init() { HAL::Motor::init(); }

/**
 * @brief Ungeregeltes Vorwärtsfahren.
 *
 * @details
 * Setzt beide Motoren auf denselben PWM-Wert.
 *
 * Didaktik:
 * Da Motoren Toleranzen haben und der Boden uneben ist, wird der Rover
 * hierbei fast immer eine leichte Kurve fahren (Drift). Das ist normal
 * für Open-Loop-Systeme.
 */
void moveForward(uint8_t speed) {
    // Beide Motoren positiv (+) -> Vorwärts
    HAL::Motor::setSpeed(speed, speed);
}

/**
 * @brief Ungeregeltes Rückwärtsfahren.
 */
void moveBackward(uint8_t speed) {
    // Beide Motoren negativ (-) -> Rückwärts
    // Der Cast auf int passiert implizit, ist hier aber für Klarheit gut:
    int pwm = -static_cast<int>(speed);
    HAL::Motor::setSpeed(pwm, pwm);
}

/**
 * @brief Pivot-Turn nach links (Drehen auf der Stelle).
 *
 * @details
 * Kinematik:
 * - Linkes Rad: Rückwärts
 * - Rechtes Rad: Vorwärts
 * -> Drehung um die vertikale Z-Achse (Gierachse) gegen den Uhrzeigersinn.
 */
void turnLeft(uint8_t speed) {
    HAL::Motor::setSpeed(-static_cast<int>(speed), static_cast<int>(speed));
}

/**
 * @brief Pivot-Turn nach rechts (Drehen auf der Stelle).
 *
 * @details
 * Kinematik:
 * - Linkes Rad: Vorwärts
 * - Rechtes Rad: Rückwärts
 * -> Drehung um die vertikale Z-Achse (Gierachse) im Uhrzeigersinn.
 */
void turnRight(uint8_t speed) {
    HAL::Motor::setSpeed(static_cast<int>(speed), -static_cast<int>(speed));
}

/**
 * @brief Sofortiger Stopp aller Antriebe.
 */
void stopMove() { HAL::Motor::stop(); }

} // namespace Logic::Motion
