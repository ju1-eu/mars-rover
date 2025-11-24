/**
 * @file       Motion.h
 * @brief      Schnittstellendefinition für die Fahrzeug-Kinematik-Steuerung.
 * @details    Definiert High-Level-Befehle für Robotermanöver.
 *             Diese Schicht übersetzt logische Bewegungen (z. B. „Vorwärts“)
 *             in spezifische Motoranweisungen an den HAL-Layer.
 * @dependency hal/Motor.h (Implementierungsdetail)
 * @author     Jan Unger
 * @version    1.0.1
 * @date       2025-11-23
 */

#pragma once

#include <stdint.h> // explizite, plattformunabhängige Ganzzahltypen

/**
 * @namespace Logic::Motion
 * @brief Beinhaltet die Steuerung der Grundbewegungen des Roboters.
 * @details Alle Funktionen sind kurz laufende, nicht-blockierende Aufrufe
 *          und steuern die Motoren direkt über den `HAL::Motor`-Layer.
 */
namespace Logic::Motion {

/**
 * @brief Initialisiert die Bewegungslogik und die zugrundeliegende
 *        Motor-Hardware.
 * @pre   Muss einmalig vor allen anderen Funktionen im `setup()` aufgerufen
 *        werden.
 */
void init();

// ----------------------------------------------------------------------
// GRUNDBEWEGUNGEN (Kinematik)
// ----------------------------------------------------------------------

/**
 * @brief Fährt den Roboter geradeaus vorwärts.
 * @param speed  Die Geschwindigkeit (PWM-Duty-Cycle).
 *               Erwarteter Wertebereich: 0–255. Werte außerhalb des
 *               zulässigen Bereichs werden in der Implementierung begrenzt.
 */
void moveForward(uint8_t speed);

/**
 * @brief Fährt den Roboter geradeaus rückwärts.
 * @param speed  Die Geschwindigkeit (PWM-Duty-Cycle, Betrag).
 * @note  Die Richtungsumkehr (Vorzeichen) wird im Implementierungs-Layer
 *        vorgenommen.
 */
void moveBackward(uint8_t speed);

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot-Turn) nach links aus.
 * @param speed  Die Rotationsgeschwindigkeit (PWM-Duty-Cycle, Betrag).
 */
void turnLeft(uint8_t speed);

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot-Turn) nach rechts aus.
 * @param speed  Die Rotationsgeschwindigkeit (PWM-Duty-Cycle, Betrag).
 */
void turnRight(uint8_t speed);

/**
 * @brief Stoppt alle Motorbewegungen.
 * @details Setzt die Geschwindigkeit der Motoren auf 0.
 * @safety  Kann als Bestandteil eines Not-Aus-Konzepts verwendet werden.
 */
void stopMove();

} // namespace Logic::Motion
