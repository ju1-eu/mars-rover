/**
 * @file       Config.h
 * @brief      Zentrale Konfigurationsparameter und physikalische Grenzwerte.
 *
 * @details
 * Dieses Header-File definiert die "Gesetze" des Rovers:
 * 1. Hardware-Limits (Safety): Was darf das System maximal tun?
 * 2. Standard-Werte (Defaults): Wie schnell soll er im Normalfall fahren?
 * 3. Test-Parameter: Wie verhält er sich im Diagnose-Modus?
 *
 * Architektur-Prinzip:
 * Durch die Bündelung in `Config::` vermeiden wir "Magic Numbers" im Code
 * (wie z. B. `motor.setSpeed(100)`). Stattdessen nutzen wir semantische
 * Namen (`Config::SpeedCruise`), was die Lesbarkeit und Sicherheit erhöht.
 *
 * @author     Jan Unger
 * @version    1.1.0 (Phase 1 Anpassung)
 * @date       2025-11-25
 */

#pragma once

#include <stdint.h>

/**
 * @namespace Config
 * @brief Globale Parameter für Hardware-Limits und Standardverhalten.
 */
namespace Config {

// ==========================================================================
// 1. HARDWARE-LIMITS (SAFETY)
// ==========================================================================

/**
 * @brief Absolute Obergrenze für PWM (Hardware-Schutz).
 *
 * @details
 * Definiert 100% Leistung (8-Bit PWM).
 * Werte über diesem Limit werden von der HAL gekappt.
 */
constexpr int SpeedMax = 255;

/**
 * @brief Definition für Stillstand.
 */
constexpr int SpeedStop = 0;

// ==========================================================================
// 2. FAHR-PHYSIK & AUTONOMIE (PHASE 1)
// ==========================================================================

/**
 * @brief Standardgeschwindigkeit für Geradeausfahrt (Cruise).
 *
 * @details
 * Ein Kompromiss aus Dynamik und Regelbarkeit.
 * - Zu schnell (>150): PID-Regler hat weniger "Headroom" zum Korrigieren
 * (da er einen Motor beschleunigen müsste, der schon fast am Limit ist).
 * - Zu langsam (<60): Haftreibung (Stiction) der Getriebe führt zu
 * ruckeligem Anfahren oder Stillstand trotz Motor-Spannung.
 *
 * @unit Duty-Cycle (0–255)
 */
constexpr uint8_t SpeedCruise = 100;

/**
 * @brief Standardgeschwindigkeit für Drehungen (Pivot Turn).
 *
 * @details
 * Sollte niedriger sein als Cruise, um:
 * 1. Schlupf zu minimieren (wichtig für Odometrie/Winkelberechnung),
 * 2. die Gyro-Sensoren nicht durch zu hohe Zentrifugalkräfte/Vibrationen
 * zu stören.
 *
 * @unit Duty-Cycle (0–255)
 */
constexpr uint8_t SpeedTurn = 80;

/**
 * @brief Sicherheitsabstand für Hindernisse.
 *
 * @details
 * Unterschreitet der Ultraschall-Sensor diesen Wert, wird ein
 * Ausweichmanöver eingeleitet.
 *
 * @unit Zentimeter (cm)
 */
constexpr float ObstacleStopDistCm = 15.0f;

// ==========================================================================
// 3. DIAGNOSE & TEST
// ==========================================================================

/**
 * @brief Dauer eines Testschritts (z. B. "Motor Links dreht").
 * @unit  Millisekunden (ms)
 */
constexpr unsigned long TestPhaseDuration = 2000UL;

/**
 * @brief PWM-Wert für reine Funktionstests (Hardware Check).
 * @details Aggressiver als Cruise, um Funktion sicherzustellen.
 */
constexpr int SpeedTest = 150;

} // namespace Config
