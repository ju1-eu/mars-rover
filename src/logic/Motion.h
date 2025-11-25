/**
 * @file       Motion.h
 * @brief      Schnittstellendefinition für die Fahrzeug-Kinematik-Steuerung.
 *
 * @details
 * Dieses Header-File definiert die High-Level-Schnittstelle für Fahrmanöver
 * eines Roboters mit Differenzialantrieb (Differential Drive). Es stellt
 * abstrakte Bewegungsbefehle wie „vorwärts“, „rückwärts“ und „Drehung auf der
 * Stelle“ bereit und bildet damit die logische Kinematik-Schicht zwischen
 * Applikation (z. B. Fahrassistent, Autonomer Modus) und Hardware-Abstraktion
 * (HAL).
 *
 * Layering:
 *  - Oben: Anwendung / Fahrlogik (z. B. Autonome Steuerung).
 *  - Mitte: Logic::Motion (dieses Modul, Kinematik-Layer).
 *  - Unten: HAL::Motor (direkte Motoransteuerung, PWM/Richtung).
 *
 * Die Implementierung der Funktionen findet sich in @c Motion.cpp und
 * verwendet intern den Motor-HAL. Aufrufende Module interagieren ausschließlich
 * mit dieser Schnittstelle und sind damit unabhängig von konkreten
 * Motortreibern oder Pinbelegungen.
 *
 * @author     Jan Unger
 * @version    1.0.1
 * @date       2025-11-23
 */

#pragma once

#include <stdint.h> // explizite, plattformunabhängige Ganzzahltypen

/**
 * @namespace Logic::Motion
 * @brief High-Level-Steuerung der Grundbewegungen des Roboters.
 *
 * @details
 * Dieses Namensraum-Bündel kapselt alle kinematischen Basisoperationen
 * eines Differenzialantriebs:
 *  - trans­latorische Bewegung (vorwärts/rückwärts),
 *  - rotatorische Bewegung (Pivot-Turn nach links/rechts),
 *  - sofortiger Stopp.
 *
 * Alle Funktionen:
 *  - sind kurz laufend und nicht-blockierend,
 *  - rufen direkt den Motor-HAL ( @c HAL::Motor ) auf,
 *  - treffen keine Entscheidungen über Sensordaten oder Sicherheitslogik
 *    (dies liegt in höheren Schichten, z. B. DriveAssistant).
 */
namespace Logic::Motion {

/**
 * @brief Initialisiert die Bewegungslogik und die zugrundeliegende
 * Motor-Hardware.
 *
 * @details
 * Reicht den Initialisierungsaufruf an den Motor-HAL weiter und stellt sicher,
 * dass die nachfolgenden Bewegungsbefehle gültig sind. Typische Aufgaben des
 * HAL-Layers sind z. B.:
 *  - Konfiguration der PWM-Kanäle,
 *  - Setzen der Pinrichtungen,
 *  - Initialisieren der H-Brücken.
 *
 * @pre
 *  - Muss genau einmal vor allen anderen Funktionen (z. B. in @c setup() )
 *    aufgerufen werden.
 * @post
 *  - Der Motor-HAL ist betriebsbereit, Bewegungsbefehle können sicher
 *    ausgeführt werden.
 */
void init();

// ----------------------------------------------------------------------
// GRUNDBEWEGUNGEN (Kinematik)
// ----------------------------------------------------------------------

/**
 * @brief Fährt den Roboter geradeaus vorwärts.
 *
 * @details
 * Setzt beide Antriebsräder auf dieselbe, positive Geschwindigkeit, sodass
 * sich das Fahrzeug (bei symmetrischer Mechanik) entlang seiner Längsachse
 * nach vorne bewegt.
 *
 * @param speed
 *  PWM-Duty-Cycle im Bereich 0–255.
 *  - 0   = Stillstand,
 *  - 255 = maximale vorwärtsgerichtete Ansteuerung gemäß HAL-/Treiberlogik.
 *
 * Werte außerhalb des zulässigen Bereichs werden in der Implementierung
 * begrenzt oder saturiert.
 */
void moveForward(uint8_t speed);

/**
 * @brief Fährt den Roboter geradeaus rückwärts.
 *
 * @details
 * Setzt beide Antriebsräder auf dieselbe, negative Geschwindigkeit (in der
 * Implementierung), sodass sich das Fahrzeug entlang seiner Längsachse nach
 * hinten bewegt.
 *
 * @param speed
 *  PWM-Duty-Cycle (Betrag) im Bereich 0–255.
 *
 * @note
 * Die eigentliche Richtungsumkehr erfolgt im Implementierungs-Layer
 * (@c Motion.cpp ), indem der Betrag in einen negativen Wert für
 * @c HAL::Motor::setSpeed() umgewandelt wird.
 */
void moveBackward(uint8_t speed);

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot-Turn) nach links aus.
 *
 * @details
 * Realisiert eine Rotation um die Hochachse des Fahrzeugs (theoretischer
 * Wenderadius 0), indem:
 *  - der linke Motor rückwärts
 *  - und der rechte Motor vorwärts angesteuert wird.
 *
 * @param speed
 *  Rotationsgeschwindigkeit (PWM-Duty-Cycle, Betrag) im Bereich 0–255.
 *
 * @note
 * Das tatsächliche Drehverhalten hängt von Bodenhaftung, Reibung und
 * Fahrwerksgeometrie ab. Bei zu hohen Werten kann Schlupf auftreten.
 */
void turnLeft(uint8_t speed);

/**
 * @brief Führt eine Drehung auf der Stelle (Pivot-Turn) nach rechts aus.
 *
 * @details
 * Realisiert eine Rotation um die Hochachse des Fahrzeugs (theoretischer
 * Wenderadius 0), indem:
 *  - der linke Motor vorwärts
 *  - und der rechte Motor rückwärts angesteuert wird.
 *
 * @param speed
 *  Rotationsgeschwindigkeit (PWM-Duty-Cycle, Betrag) im Bereich 0–255.
 */
void turnRight(uint8_t speed);

/**
 * @brief Stoppt alle Motorbewegungen.
 *
 * @details
 * Setzt die angeforderte Geschwindigkeit beider Motoren auf 0 und delegiert
 * den Befehl an den Motor-HAL. Ob dies als:
 *  - passives Ausrollen (Coasting) oder
 *  - aktives Bremsen (Braking)
 * umgesetzt wird, hängt von der Implementierung von @c HAL::Motor::stop()
 * ab.
 *
 * @safety
 * Diese Funktion kann (in Kombination mit Sensorik und Logikschicht) als
 * Bestandteil eines Not-Aus-Konzepts verwendet werden, z. B. bei:
 *  - Kippen des Fahrzeugs,
 *  - Hindernis-Erkennung,
 *  - manueller Not-Aus-Betätigung.
 */
void stopMove();

} // namespace Logic::Motion
