/**
 * @file       Config.h
 * @brief      Zentrale Konfigurationsparameter und Systemkonstanten.
 * @details    Enthält Parameter für Geschwindigkeitsbegrenzungen (Safety)
 *             sowie Zeit- und Testparameter für die Hardware-Diagnose
 *             und spätere Applikationen.
 * @author     Jan Unger
 * @version    1.0.1
 * @date       2025-11-23
 */

#pragma once

#include <stdint.h> // explizite, plattformunabhängige Ganzzahltypen

/**
 * @namespace Config
 * @brief Globale, schreibgeschützte Parameter des Robotersystems.
 */
namespace Config {

// ----------------------------------------------------------------------
// SICHERHEIT & HARDWARE-GRENZEN
// ----------------------------------------------------------------------

/**
 * @brief Maximal erlaubter PWM-Wert.
 * @details Definiert die physikalische Obergrenze des Duty-Cycles (8-Bit-PWM).
 * @unit   Duty-Cycle (0–255)
 * @safety Sollte nicht überschritten werden, um Überlastung der Motoren/Treiber
 *         zu vermeiden.
 */
constexpr int SpeedMax = 255;

/**
 * @brief Definierter Wert für den Stillstand.
 * @details Dient der Klarheit, falls später eine aktive Bremsung
 *          (z. B. 1) definiert wird.
 */
constexpr int SpeedStop = 0;

// ----------------------------------------------------------------------
// DIAGNOSE & TESTPARAMETER
// ----------------------------------------------------------------------

/**
 * @brief Dauer eines einzelnen Testschritts in der Hardware-Diagnose.
 * @details Wird z. B. von Motor-Test-Routinen verwendet, um die Drehrichtung
 *          optisch prüfen zu können (Lektion 4/5 – Hardware-Test).
 * @unit    Millisekunden (ms)
 */
constexpr unsigned long TestPhaseDuration = 2000UL; // 2 Sekunden pro Schritt

/**
 * @brief PWM-Geschwindigkeit, die für Hardware-Tests verwendet wird.
 * @details Ein mittlerer Wert, um Motoren klar drehen zu lassen, aber
 *          Lärm- und Unfallgefahr zu minimieren.
 * @unit    Duty-Cycle (0–255)
 */
constexpr int SpeedTest = 150;

// TODO: Einen separaten Wert für die Laufzeit-Applikation (z. B.
//       CruiseSpeed) hinzufügen, falls die Dauergeschwindigkeit
//       von der Testgeschwindigkeit abweichen soll.

} // namespace Config
