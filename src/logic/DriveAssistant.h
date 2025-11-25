/**
 * @file       DriveAssistant.h
 * @brief      Schnittstelle für Spurhalte- und Kippschutz-Assistent.
 *
 * @details
 * Deklariert die Initialisierungs- und Update-Funktion des Fahrassistenten.
 * Der Assistent nutzt IMU-Daten (Pitch, Gierrate), um:
 * - einen maximalen Kippwinkel zu überwachen (Kippschutz),
 * - bei Geradeausfahrt Gier-Abweichungen aktiv zu korrigieren.
 *
 * Änderung (Phase 1.1):
 * Die Regelung erfolgt nun über einen PID-Regler (statt reinem P-Regler),
 * um stationäre Fehler (z. B. durch Teppich-Drift) und Oszillationen
 * zu minimieren.
 *
 * Die Implementierung findet sich in @c DriveAssistant.cpp und verwendet
 * intern @c Logic::PidController, @c HAL::Sensor und @c HAL::Motor.
 */

#pragma once

#include <stdint.h> // explizite, plattformunabhängige Ganzzahltypen

namespace Logic::DriveAssistant {

/**
 * @brief Initialisiert den Assistenten und setzt Regler-Zustände zurück.
 *
 * @details
 * Setzt den internen PID-Regler (Integral-Speicher, letzter Fehler) sowie
 * die Zeitmessung für die Delta-t-Berechnung zurück.
 *
 * @note
 * Sollte einmalig im @c setup() der Anwendung aufgerufen werden, sowie
 * immer dann, wenn eine neue Mission startet, um "Integral Windup"
 * aus vorherigen Fahrten zu verhindern.
 */
void init();

/**
 * @brief Führt Spurhalte- und Kippschutzlogik aus und steuert die Motoren.
 *
 * @details
 * Typischer Aufrufzyklus (empfohlen ca. 10-50 Hz):
 * 1. Berechnet die Zeitdifferenz (@f$ \Delta t @f$) seit dem letzten Aufruf.
 * 2. Liest IMU-Daten über @c HAL::Sensor (Pitch, Gierrate).
 * 3. Prüft den Pitch-Winkel gegen @c MAX_PITCH_DEG (Kippschutz).
 * 4. Berechnet via PID-Algorithmus die nötige Korrektur, um die
 * Gierrate auf 0 zu halten.
 * 5. Mischt das Korrektursignal auf die Basisgeschwindigkeit und steuert
 * via @c HAL::Motor::setSpeed().
 *
 * @param baseSpeed
 * Basisgeschwindigkeit (PWM-Sollwert) für beide Motoren im Bereich 0–255.
 * Von diesem Wert aus wird nach links/rechts differenziell korrigiert.
 *
 * @retval true
 * Kippwinkel innerhalb des sicheren Bereichs; Motoren wurden geregelt.
 * @retval false
 * Maximaler zulässiger Kippwinkel überschritten (Kippgefahr).
 * Der Assistent greift nicht mehr regelnd ein (bzw. fordert Stopp);
 * der Aufrufer sollte einen Not-Halt auslösen.
 */
bool update(uint8_t baseSpeed);

} // namespace Logic::DriveAssistant
