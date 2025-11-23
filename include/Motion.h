/**
 * @file       Motion.h
 * @brief      Schnittstellendefinition für die Fahrzeug-Kinematik-Steuerung.
 * @details    Definiert High-Level-Befehle für die Robotermanöver.
 * Diese Schicht übersetzt logische Bewegungen (z.B. "Vorwärts") in
 * spezifische Motoranweisungen an den HAL-Layer.
 * @dependency hal/Motor.h (Implementierungsdetail)
 * @author     Jan Unger
 * @version    1.0.0
 * @date       2025-11-22
 */

#pragma once
#include <Arduino.h> // Notwendig für die `uint8_t` Definition, falls nicht implizit
#include <stdint.h>

/**
 * @namespace Logic::Motion
 * @brief Beinhaltet die Steuerung der Grundbewegungen des Roboters.
 * @details Alle Funktionen sind **nicht-blockierend** und steuern die
 * Motoren direkt über den `HAL::Motor` Layer.
 */
namespace Logic::Motion {

/**
 * @brief Initialisiert die Bewegungslogik und die zugrundeliegende
 * Motor-Hardware.
 * @pre   Muss einmalig vor allen anderen Funktionen im `setup()` aufgerufen
 * werden.
 */
void init();

// ----------------------------------------------------------------------
// GRUNDBEWEGUNGEN (Kinematik)
// ----------------------------------------------------------------------

/**
 * @brief Fährt den Roboter geradeaus vorwärts.
 * @param speed  Die Geschwindigkeit (PWM-Duty-Cycle).
 * Wertebereich: [0...255]. Werte > 255 werden ignoriert oder abgeschnitten.
 */
void moveForward(uint8_t speed);

/**
 * @brief Fährt den Roboter geradeaus rückwärts.
 * @param speed  Die Geschwindigkeit (PWM-Duty-Cycle, Betrag).
 * @note   Die Richtungsumkehr (`-speed`) wird im Implementierungs-Layer
 * vorgenommen.
 */
void moveBackward(uint8_t speed);

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot-Turn) nach links aus.
 * @param speed  Die Rotationsgeschwindigkeit.
 */
void turnLeft(uint8_t speed);

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot-Turn) nach rechts aus.
 * @param speed  Die Rotationsgeschwindigkeit.
 */
void turnRight(uint8_t speed);

/**
 * @brief Stoppt alle Motorbewegungen.
 * @details Setzt die Geschwindigkeit der Motoren auf 0.
 * @safety Muss als Teil des Not-Aus-Konzepts betrachtet werden.
 */
void stopMove();
} // namespace Logic::Motion
