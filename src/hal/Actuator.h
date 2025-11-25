/**
 * @file       Actuator.h
 * @brief      Schnittstelle für Zusatz-Aktoren (Servo & Licht).
 *
 * @details
 * Dieses Modul abstrahiert die Ansteuerung von Komponenten, die nicht
 * direkt zum Fahrwerk gehören.
 *
 * Architektur-Einordnung & Didaktik:
 * - **Abstraction Layer:** Der Aufrufer muss nicht wissen, dass der Servo
 * an Pin 6 hängt oder dass die LEDs via SoftPWM gesteuert werden.
 * - **Safety Layer:** Die Funktion `setCameraAngle` verhindert mechanische
 * Schäden durch Software-Limits ("Clamping").
 *
 * @author     Jan Unger
 * @version    1.0.2
 * @date       2025-11-25
 */

#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace HAL::Actuator {

/**
 * @brief Initialisiert Servo und LED-Treiber.
 *
 * @details
 * - Startet SoftPWM.
 * - Fährt den Servo in Neutralposition (90°).
 * - Schaltet alle LEDs aus.
 *
 * @pre Muss im setup() aufgerufen werden.
 */
void init();

/**
 * @brief Bewegt die Kamera sicher auf einen Winkel.
 *
 * @details
 * Implementiert "Clamping" (Sättigung):
 * Ein angeforderter Winkel von 0° oder 180° wird intern auf den
 * mechanisch sicheren Bereich (z.B. 20° - 140°) begrenzt.
 *
 * @param angleDeg Zielwinkel in Grad (0-180).
 */
void setCameraAngle(int angleDeg);

// ----------------------------------------------------------------------
// DIAGNOSE-FUNKTIONEN (Nicht-blockierend)
// ----------------------------------------------------------------------

/**
 * @brief Testet die RGB-LEDs (Rot -> Grün -> Blau).
 *
 * @details
 * Funktion ist **nicht-blockierend**: Sie nutzt intern `millis()`, um
 * den Zustand nur einmal pro Sekunde zu wechseln.
 *
 * @note Muss zyklisch (z.B. in loop) aufgerufen werden, damit der
 * Wechsel stattfindet.
 */
void rgbTest();

/**
 * @brief Testet den Servo (Langsamer Sweep).
 *
 * @details
 * Funktion ist **nicht-blockierend**: Bewegt den Servo alle paar
 * Millisekunden um einen kleinen Schritt.
 *
 * @note Dient der Prüfung, ob der Servo mechanisch frei läuft.
 */
void servoTest();

} // namespace HAL::Actuator
