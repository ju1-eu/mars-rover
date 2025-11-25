/**
 * @file       Motion.h
 * @brief      Schnittstelle für die "blinde" Kinematik (Open-Loop).
 *
 * @details
 * Dieses Modul definiert die abstrakten Bewegungsbefehle für einen
 * Differenzialantrieb (Differential Drive).
 *
 * Architektur-Einordnung & Didaktik:
 * - **Ebene:** Kinematik-Layer (Logic).
 * - **Prinzip:** Open-Loop (Steuerung). Es gibt KEINE Rückkopplung.
 * Der Befehl "Fahre geradeaus" sendet an beide Motoren denselben Wert.
 * Ob der Roboter aufgrund von Bodenunebenheiten oder Motor-Toleranzen
 * tatsächlich geradeaus fährt, wird hier NICHT geprüft.
 *
 * Abgrenzung:
 * Für geregeltes Fahren (Closed-Loop), das Sensordaten nutzt um die Spur
 * zu halten, ist der @c Logic::DriveAssistant zuständig.
 *
 * @author     Jan Unger
 * @version    1.0.1
 * @date       2025-11-25
 */

#pragma once

#include <stdint.h> // Für uint8_t

namespace Logic::Motion {

/**
 * @brief Initialisiert die Motor-Treiber.
 *
 * @details
 * Leitet den Aufruf an @c HAL::Motor::init() weiter. Muss einmalig im
 * @c setup() gerufen werden.
 */
void init();

// ----------------------------------------------------------------------
// BEWEGUNGS-PRIMITIVE (Grundbausteine)
// ----------------------------------------------------------------------

/**
 * @brief Ungeregeltes Vorwärtsfahren.
 *
 * @details
 * Setzt beide Motoren auf positive Drehrichtung mit gleicher PWM-Rate.
 *
 * @param speed
 * Geschwindigkeit (0–255).
 * @note Bei niedrigen Werten (<60) reicht das Drehmoment oft nicht aus,
 * um die Haftreibung zu überwinden (Deadband).
 */
void moveForward(uint8_t speed);

/**
 * @brief Ungeregeltes Rückwärtsfahren.
 *
 * @details
 * Setzt beide Motoren auf negative Drehrichtung (invertiert).
 *
 * @param speed Geschwindigkeit (0–255).
 */
void moveBackward(uint8_t speed);

/**
 * @brief Pivot-Turn (Tellerwende) nach Links.
 *
 * @details
 * Dreht den Roboter auf der Stelle gegen den Uhrzeigersinn.
 * Kinematik: Links rückwärts / Rechts vorwärts.
 *
 * @param speed Drehgeschwindigkeit (0–255).
 */
void turnLeft(uint8_t speed);

/**
 * @brief Pivot-Turn (Tellerwende) nach Rechts.
 *
 * @details
 * Dreht den Roboter auf der Stelle im Uhrzeigersinn.
 * Kinematik: Links vorwärts / Rechts rückwärts.
 *
 * @param speed Drehgeschwindigkeit (0–255).
 */
void turnRight(uint8_t speed);

/**
 * @brief Sofortiger Stopp (Hard Stop).
 *
 * @details
 * Setzt PWM auf 0. Das physikalische Verhalten (Rollen vs. Bremsen)
 * hängt vom Motortreiber (Decay Mode) ab.
 */
void stopMove();

} // namespace Logic::Motion
