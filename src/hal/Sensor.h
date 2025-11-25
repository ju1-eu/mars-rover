/**
 * @file       Sensor.h
 * @brief      Schnittstelle für die Wahrnehmung (Perzeption).
 *
 * @details
 * Dieses Modul bündelt alle Sensoren.
 *
 * Architektur-Einordnung & Didaktik:
 * - **Polling-Prinzip:** Funktionen wie `ultrasonicUpdate()` müssen zyklisch
 * gerufen werden. Sie "holen" die Daten und speichern sie zwischen.
 * - **Getter-Prinzip:** Funktionen wie `getUltrasonicDistance()` liefern
 * sofort den letzten bekannten Wert, ohne Zeit zu verbrauchen.
 * -> Das entkoppelt die langsame Physik (Schallgeschwindigkeit) von der
 * schnellen Entscheidungslogik.
 *
 * @author     Jan Unger
 * @version    1.1.0
 * @date       2025-11-25
 */

#pragma once

#include <Arduino.h>

namespace HAL::Sensor {

// ==========================================================================
// 1. INITIALISIERUNG
// ==========================================================================

/**
 * @brief Startet alle Sensoren.
 *
 * @details
 * - IMU: Verbindungstest und Wake-Up.
 * - IR: Pins auf INPUT.
 * - Ultraschall: Pins vorbereiten.
 *
 * @pre Muss einmalig im setup() gerufen werden.
 */
void init();

// ==========================================================================
// 2. ULTRASCHALL (Distanz)
// ==========================================================================

/**
 * @brief Führt eine Messung durch (Zeitintensiv!).
 *
 * @details
 * Sendet Ping und wartet auf Echo.
 * ACHTUNG: Ist nicht-blockierend implementiert (misst nur alle 100ms),
 * aber wenn gemessen wird, wartet die CPU kurz auf das Echo (µs-Bereich).
 *
 * @note Zyklisch aufrufen!
 */
void ultrasonicUpdate();

/**
 * @brief Liefert den letzten Messwert sofort zurück.
 *
 * @return Distanz in cm (oder 999.0 wenn nichts erkannt).
 */
float getUltrasonicDistance();

/**
 * @brief Debug-Funktion: Gibt Werte auf Serial aus.
 */
void ultrasonicTest();

// ==========================================================================
// 3. INFRAROT (Hindernis/Abgrund)
// ==========================================================================

/**
 * @brief Linker IR-Sensor (Digital).
 * @return true = Hindernis erkannt / Boden reflektiert.
 */
bool irLeftBlocked();

/**
 * @brief Rechter IR-Sensor (Digital).
 * @return true = Hindernis erkannt / Boden reflektiert.
 */
bool irRightBlocked();

/**
 * @brief Debug-Funktion: Zeigt IR-Status live an.
 */
void irTest();

// ==========================================================================
// 4. BATTERIE (Energie)
// ==========================================================================

/**
 * @brief Misst die Akkuspannung.
 * @return Spannung in Volt (z.B. 7.4).
 */
float getBatteryVoltage();

/**
 * @brief Berechnet den Füllstand.
 * @return 0-100 Prozent.
 */
uint8_t getBatteryPercentage();

// ==========================================================================
// 5. IMU (Lage & Bewegung)
// ==========================================================================

/**
 * @brief Kalibriert die Nullpunkte (Offsets).
 *
 * @details
 * Der Rover muss 1-2 Sekunden absolut still stehen.
 * Berechnet den Durchschnittswert des Rauschens und speichert ihn als
 * Nullpunkt.
 *
 * @param samples Anzahl der Messungen (Standard: 500).
 */
void imuCalibrate(uint16_t samples = 500);

/**
 * @brief Liest Sensoren, filtert Rauschen und berechnet Winkel.
 *
 * @details
 * Hier läuft der Komplementärfilter (Sensor Fusion).
 * Muss so oft wie möglich (mind. 50Hz) gerufen werden, damit das
 * Integral (Winkel aus Rate) präzise bleibt.
 */
void imuUpdate();

// --- Getter (Liefern gefilterte Werte) ---

/**
 * @brief Neigung nach Vorne/Hinten (Pitch).
 * @return Grad (Positiv = Nase hoch).
 */
float getPitch();

/**
 * @brief Neigung zur Seite (Roll).
 * @return Grad.
 */
float getRoll();

/**
 * @brief Drehgeschwindigkeit um die Hochachse (Gierrate).
 * @return Grad pro Sekunde (°/s).
 */
float getYawRate();

/**
 * @brief Debug-Funktion: Zeigt Lage-Daten live an.
 */
void imuTest();

} // namespace HAL::Sensor
