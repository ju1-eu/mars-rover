/**
 * @file       Motor.h
 * @brief      Schnittstellendefinition (API) für den Motor-Treiber.
 * @details    Stellt Funktionen zur direkten Ansteuerung der H-Brücke bereit.
 *             Abstrahiert die spezifischen Pin-Zuweisungen und PWM-Bibliotheken
 *             (SoftPWM) für einen Differenzialantrieb.
 */

#pragma once
#include <Arduino.h>
#include <stdint.h>

namespace HAL::Motor {

/**
 * @brief  Konfiguriert die GPIO-Pins und startet die PWM-Timer.
 * @pre    Muss einmalig im `setup()` aufgerufen werden.
 * @post   Motoren sind im sicheren Zustand (Stopp).
 */
void init();

/**
 * @brief  Setzt die Motorgeschwindigkeit und -richtung (Open Loop).
 *
 * @param  leftSpeed   Sollgeschwindigkeit für den linken Motor.
 *                     Vorzeichen: < 0 rückwärts, 0 Stopp (Coasting), > 0
 * vorwärts.
 * @param  rightSpeed  Sollgeschwindigkeit für den rechten Motor.
 *                     Gleiche Konvention wie bei leftSpeed.
 *
 * @note   Die Implementierung begrenzt die Werte intern auf den zulässigen
 *         Bereich [-Config::SpeedMax ... +Config::SpeedMax] (Clamping),
 *         bevor sie an die SoftPWM-Bibliothek übergeben werden.
 */
void setSpeed(int leftSpeed, int rightSpeed);

/**
 * @brief  Schaltet die Motoren sofort ab.
 *
 * @details Setzt beide Motorkanäle auf PWM = 0.
 *          Die Motoren rollen aus (kein aktives Bremsen).
 *
 * @safety  Sollte in Not-Aus-Routinen oder bei Kommunikationsverlust aufgerufen
 *          werden.
 */
void stop();

/**
 * @brief  Nicht-blockierender Motor-Selbsttest.
 *
 * @details Wird zyklisch aus `loop()` (Phase MOTOR_TEST) aufgerufen und
 *          durchläuft in 1-s-Schritten folgende Sequenz:
 *          0: Vorwärts, 1: Rückwärts, 2: Linksdrehung auf der Stelle,
 *          3: Rechtsdrehung auf der Stelle, 4: Stopp.
 *          Die Zeitsteuerung erfolgt über `millis()`, es wird kein `delay()`
 *          verwendet.
 */
void motorTestLogic();

} // namespace HAL::Motor
