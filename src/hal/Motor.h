/**
 * @file       Motor.h
 * @brief      Schnittstelle für den Hardware-Treiber (H-Brücke).
 *
 * @details
 * Dieses Modul bildet die unterste Ebene der Antriebssteuerung (HAL).
 *
 * Architektur-Einordnung & Didaktik:
 * - **Open Loop:** Diese Ebene führt Befehle "blind" aus. Sie weiß nicht,
 * ob der Roboter tatsächlich fährt oder ob er gegen eine Wand drückt.
 * - **Abstraktion:** Versteckt die Komplexität der SoftPWM-Bibliothek
 * und der Pin-Manipulationen vor dem Rest des Systems.
 * - **Sicherheit:** Implementiert die Not-Halt-Funktion `stop()`.
 *
 * @author     Jan Unger
 * @version    1.1.0
 * @date       2025-11-25
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace HAL::Motor {

/**
 * @brief Initialisiert die Motor-Hardware.
 *
 * @details
 * - Konfiguriert Pins als Ausgänge.
 * - Startet die SoftPWM-Timer.
 * - Setzt Motoren in den sicheren Zustand (Stop).
 *
 * @pre Muss einmalig im setup() gerufen werden.
 */
void init();

/**
 * @brief Setzt die Geschwindigkeit (PWM) direkt.
 *
 * @details
 * Dies ist der "Gashebel" des Roboters.
 *
 * @param leftSpeed  Geschwindigkeit Links (-255 bis +255).
 * @param rightSpeed Geschwindigkeit Rechts (-255 bis +255).
 *
 * @note Werte > 255 oder < -255 werden automatisch auf das Maximum
 * begrenzt (Clamping gemäß Config::SpeedMax).
 */
void setSpeed(int leftSpeed, int rightSpeed);

/**
 * @brief Not-Halt (Coasting).
 *
 * @details
 * Schaltet die Spannung an den Motoren sofort ab.
 * Der Roboter rollt aus (kein aktives Bremsen).
 *
 * @safety Sollte bei jedem erkannten Fehlerzustand (Kippen, Sensor-Ausfall)
 * gerufen werden.
 */
void stop();

/**
 * @brief Diagnose-Funktion: Motor-Selbsttest.
 *
 * @details
 * Eine nicht-blockierende Routine, die nacheinander alle Bewegungsrichtungen
 * abfährt. Dient dazu, Verdrahtungsfehler (z.B. linker Motor dreht falsch
 * herum) schnell zu finden.
 *
 * @note Muss zyklisch in der Loop aufgerufen werden.
 */
void motorTestLogic();

} // namespace HAL::Motor
