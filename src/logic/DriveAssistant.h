/**
 * @file       DriveAssistant.h
 * @brief      Schnittstelle für Spurhalte- und Kippschutz-Assistent.
 *
 * @details
 * Deklariert die Initialisierungs- und Update-Funktion des Fahrassistenten.
 * Der Assistent nutzt IMU-Daten (Pitch, Gierrate), um:
 *  - einen maximalen Kippwinkel zu überwachen (Kippschutz),
 *  - bei Geradeausfahrt Gier-Abweichungen aktiv zu korrigieren
 *    (Differentiallenkung über den Motor-HAL).
 *
 * Die Implementierung findet sich in @c DriveAssistant.cpp und verwendet
 * intern @c HAL::Sensor und @c HAL::Motor.
 */

#pragma once

#include <stdint.h> // explizite, plattformunabhängige Ganzzahltypen

namespace Logic::DriveAssistant {

/**
 * @brief Setzt interne Parameter des DriveAssistant zurück.
 *
 * @details
 * Aktuell dient diese Funktion als Initialisierungshaken und kann genutzt
 * werden, um interne Zustände (z. B. Filter, Flags) zurückzusetzen.
 *
 * @note
 * Sollte einmalig im @c setup() der Anwendung aufgerufen werden, bevor
 * @c update() zyklisch verwendet wird.
 */
void init();

/**
 * @brief Führt Spurhalte- und Kippschutzlogik aus und steuert die Motoren.
 *
 * @details
 * Typischer Aufrufzyklus:
 *  - Liest IMU-Daten über @c HAL::Sensor (Pitch, Gierrate),
 *  - prüft den Pitch-Winkel gegen einen Grenzwert (Kippschutz),
 *  - berechnet aus der Gierrate eine P-Regler-Korrektur,
 *  - passt die linke und rechte Motorgeschwindigkeit relativ zur
 *    Basisgeschwindigkeit an und ruft @c HAL::Motor::setSpeed() auf.
 *
 * Die Funktion ist nicht-blockierend und für den Aufruf in der Hauptschleife
 * ( @c loop() ) vorgesehen.
 *
 * @param baseSpeed
 *  Basisgeschwindigkeit (PWM-Sollwert) für beide Motoren im Bereich 0–255.
 *  Von diesem Wert aus wird nach links/rechts differenziell korrigiert.
 *
 * @retval true
 *   Kippwinkel innerhalb des sicheren Bereichs; Motoren wurden entsprechend
 *   der Reglerlogik angesteuert.
 * @retval false
 *   Maximaler zulässiger Kippwinkel überschritten (Kippgefahr). In diesem
 *   Fall sollte der Aufrufer einen Not-Stopp auslösen (z. B. Motor-Stop).
 */
bool update(uint8_t baseSpeed);

} // namespace Logic::DriveAssistant
