/**
 * @file       Config.h
 * @brief      Zentrale Konfigurationsparameter und Systemkonstanten.
 * @details    Enthält Parameter für Geschwindigkeitsbegrenzungen (Safety)
 * und Timeouts (Applikation).
 * @author     Jan Unger
 * @version    1.0.0
 * @date       2025-11-22
 */

#pragma once
#include <stdint.h> // Notwendig für die explizite Verwendung von Datentypen

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
 * @details Definiert die physikalische Obergrenze des Duty-Cycles (8-Bit PWM).
 * @unit   Duty-Cycle (0-255)
 * @safety Sollte nicht überschritten werden, um Überlastung der Motoren/Treiber
 * zu vermeiden.
 */
constexpr int SpeedMax = 255;

/**
 * @brief Definierter Wert für den Stillstand.
 * @details Dient der Klarheit, falls später eine aktive Bremsung (z.B. 1)
 * definiert wird.
 */
constexpr int SpeedStop = 0;

// ----------------------------------------------------------------------
// APPLIKATION & TESTPARAMETER
// ----------------------------------------------------------------------

/**
 * @brief Dauer der einzelnen Testphasen im HardwareTest.cpp.
 * @details Muss lang genug sein, um eine optische Prüfung der Drehrichtung
 * durch einen Techniker zu ermöglichen.
 * @unit   Millisekunden (ms)
 * @test   Verwendung in Lektion 4 (Hardware-Test).
 */
constexpr unsigned long TestPhaseDuration = 2000; // 2 Sekunden pro Schritt

/**
 * @brief Geschwindigkeit, die für Hardware-Tests verwendet wird.
 * @details Ein mittlerer Wert, um Motoren klar drehen zu lassen, aber
 * die Lärm- und Unfallgefahr zu minimieren.
 * @unit   Duty-Cycle (0-255)
 */
constexpr int SpeedTest = 150;

// TODO: Einen separaten Wert für die Laufzeit-Applikation (Lektion 5)
// hinzufügen, falls die Dauergeschwindigkeit von der Testgeschwindigkeit
// abweichen soll.

} // namespace Config
