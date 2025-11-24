/**
 * @file    Actuator.h
 * @brief   HAL-Schnittstelle für Aktoren (Servo, RGB-LEDs).
 * @details Stellt Funktionen zur Initialisierung und zum Testen von
 *          Kameraservo und RGB-LED-Streifen bereit.
 */

#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <stdint.h>

namespace HAL::Actuator {

/**
 * @brief Initialisiert Servo (Kamera) und RGB-LEDs.
 */
void init();

/**
 * @brief RGB-LED-Testsequenz (Rot → Grün → Blau).
 */
void rgbTest();

/**
 * @brief Setzt den Kameraservo auf einen bestimmten Winkel [°].
 */
void setCameraAngle(int angleDeg);

/**
 * @brief Nicht-blockierender Servotest (Sweep zwischen Min/Max).
 */
void servoTest();

} // namespace HAL::Actuator

#endif // ACTUATOR_H
