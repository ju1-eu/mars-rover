/**
 * @file       Config.h
 * @brief      Zentrale Konfigurationsparameter und Systemkonstanten.
 *
 * @details
 * Dieses Header-File bündelt alle globalen, schreibgeschützten Parameter,
 * die für das Rover-System relevant sind. Es dient als zentrale Stelle
 * für:
 *  - Geschwindigkeitsbegrenzungen (Safety),
 *  - Standardwerte für Stillstand und Testbetrieb,
 *  - Zeitparameter für Hardware-Diagnosen und Lernlektionen.
 *
 * Ziel:
 *  - Klare Trennung von Konfiguration und Logik,
 *  - einfache Wartbarkeit (Änderungen an einem Ort),
 *  - konsistente Nutzung von Grenzwerten in allen Modulen
 *    (z. B. Motor-Test, Autonomer Modus, Fahrassistent).
 *
 * @author     Jan Unger
 * @version    1.0.1
 * @date       2025-11-23
 */

#pragma once

#include <stdint.h> // explizite, plattformunabhängige Ganzzahltypen

/**
 * @namespace Config
 * @brief Globale, schreibgeschützte Parameter des Robotersystems.
 *
 * @details
 * Alle Konstanten in diesem Namensraum sind als @c constexpr definiert
 * und werden zur Compile-Zeit ausgewertet. Sie sollen:
 *  - als System-Grenzwerte (Safety Limits) dienen,
 *  - als Standardwerte für Diagnose- und Testabläufe verwendet werden,
 *  - NICHT zur Laufzeit verändert werden.
 */
namespace Config {

// ----------------------------------------------------------------------
// SICHERHEIT & HARDWARE-GRENZEN
// ----------------------------------------------------------------------

/**
 * @brief Maximal erlaubter PWM-Wert für die Motoransteuerung.
 *
 * @details
 * Definiert die physikalische Obergrenze des Duty-Cycles beim Einsatz
 * einer 8-Bit-PWM:
 *  - 0   = 0 % (aus),
 *  - 255 = 100 % (volle Ansteuerung).
 *
 * Alle Module, die Motoren ansteuern, sollten diesen Wert als
 * absolute Obergrenze verwenden (Sättigung), um Überlastungen der
 * Motoren und Treiberstufen zu vermeiden.
 *
 * @unit    Duty-Cycle (0–255)
 * @safety  Ein Erhöhen dieses Werts kann thermische und elektrische
 *          Überlastung begünstigen und sollte nur bewusst erfolgen.
 */
constexpr int SpeedMax = 255;

/**
 * @brief Definierter PWM-Wert für Stillstand.
 *
 * @details
 * Dieser Wert dient der semantischen Klarheit im Code. Falls später eine
 * Unterscheidung zwischen:
 *  - „Coast“ (Motoren stromlos, ausrollen) und
 *  - „Brake“ (aktive Bremsung über H-Brücke)
 * eingeführt wird, kann hier eine entsprechende Kodierung erfolgen.
 *
 * @unit Duty-Cycle (0–255)
 */
constexpr int SpeedStop = 0;

// ----------------------------------------------------------------------
// DIAGNOSE & TESTPARAMETER
// ----------------------------------------------------------------------

/**
 * @brief Dauer eines einzelnen Testschritts in der Hardware-Diagnose.
 *
 * @details
 * Wird z. B. in Motor-Test-Routinen genutzt, um:
 *  - Drehrichtung,
 *  - Verkabelung,
 *  - grundlegende Funktion
 * optisch oder akustisch prüfen zu können (Lektion 4/5 – Hardware-Test).
 *
 * Typische Verwendung:
 *  - Schritt 0: Vorwärts
 *  - Schritt 1: Rückwärts
 *  - Schritt 2: Links
 *  - Schritt 3: Rechts
 *
 * @unit Millisekunden (ms)
 */
constexpr unsigned long TestPhaseDuration = 2000UL; // 2 Sekunden pro Schritt

/**
 * @brief Standard-PWM-Geschwindigkeit für Hardware-Tests.
 *
 * @details
 * Ein mittlerer Wert, der:
 *  - Motoren klar erkennbar drehen lässt,
 *  - aber Geräuschpegel und mechanische Belastung begrenzt.
 *
 * Typische Einsatzszenarien:
 *  - Motor-Funktionstest auf dem Tisch,
 *  - einfache Fahrversuche im Nahbereich,
 *  - Diagnoseprogramme ohne hohe Anforderung an Dynamik.
 *
 * @unit Duty-Cycle (0–255)
 */
constexpr int SpeedTest = 150;

// TODO:
//  - Einen separaten Wert für die Laufzeit-Applikation (z. B. CruiseSpeed)
//    hinzufügen, falls die Dauergeschwindigkeit von der Testgeschwindigkeit
//    abweichen soll.

} // namespace Config
