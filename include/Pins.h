/**
 * @file       Pins.h
 * @brief      Zentrale Definition aller GPIO-Pin-Zuordnungen (Mapping).
 *
 * @details
 * Diese Datei ist das Wörterbuch zwischen Software (Logische Namen) und
 * Hardware (Physische Pin-Nummern am Mikrocontroller).
 *
 * Warum machen wir das?
 * 1. Lesbarkeit: `digitalWrite(Pin::MotorL_Forward, ...)` ist verständlicher
 * als `digitalWrite(2, ...)`.
 * 2. Wartbarkeit: Ändert sich der Anschluss, muss nur hier EINE Zahl geändert
 * werden.
 *
 * @hardware
 * - Controller: Arduino Uno / SunFounder R3-kompatibel
 * - Shield:     GalaxyRVR Shield
 *
 * @author     Jan Unger
 * @version    1.0.2
 * @date       2025-11-25
 */

#pragma once

#include <Arduino.h> // Definiert uint8_t und Pin-Namen (A0, D1 etc.)

/**
 * @namespace Pin
 * @brief Kapselt die Hardware-Adressen.
 */
namespace Pin {

// ==========================================================================
// 1. ANTRIEB (MOTOREN & TREIBER)
// ==========================================================================

/**
 * @brief Linker Motor: Vorwärts (IN1).
 * @unit  Digital Pin (PWM fähig per SoftPWM)
 */
constexpr uint8_t MotorL_Forward = 2;

/**
 * @brief Linker Motor: Rückwärts (IN2).
 * @unit  Digital Pin (Hardware PWM)
 */
constexpr uint8_t MotorL_Reverse = 3;

/**
 * @brief Rechter Motor: Vorwärts (IN3).
 * @unit  Digital Pin (Hardware PWM)
 */
constexpr uint8_t MotorR_Forward = 5;

/**
 * @brief Rechter Motor: Rückwärts (IN4).
 * @unit  Digital Pin (PWM fähig per SoftPWM)
 */
constexpr uint8_t MotorR_Reverse = 4;

// ==========================================================================
// 2. SENSOREN (WAHRNEHMUNG)
// ==========================================================================

/**
 * @brief Ultraschall: Trigger-Signal (Senden).
 * @note  Pin D10 wird im Code dynamisch zwischen INPUT und OUTPUT geschaltet,
 * da der Sensor oft nur mit einer Leitung angeschlossen ist.
 */
constexpr uint8_t Ultrasonic_Trig = 10;

/**
 * @brief Ultraschall: Echo-Signal (Empfangen).
 */
constexpr uint8_t Ultrasonic_Echo = 10;

/**
 * @brief Infrarot-Sensor Links (Absturz/Linie).
 * @details LOW = Reflektion (Boden/Hindernis), HIGH = Schwarz/Abgrund.
 */
constexpr uint8_t IR_Left = 8;

/**
 * @brief Infrarot-Sensor Rechts.
 */
constexpr uint8_t IR_Right = 7;

/**
 * @brief Batteriespannungsmessung (Analog).
 * @details Messung über Spannungsteiler.
 */
constexpr uint8_t Battery = A3;

// ==========================================================================
// 3. AKTOREN (LICHT & BEWEGUNG)
// ==========================================================================

/**
 * @brief Servo für Kamera-Neigung (Tilt).
 */
constexpr uint8_t Servo = 6;

/**
 * @brief RGB LED: Rot-Kanal.
 */
constexpr uint8_t RGB_R = 12;

/**
 * @brief RGB LED: Grün-Kanal.
 * @warning Verbunden mit Onboard-LED (L). Leuchtet beim Booten oft auf.
 */
constexpr uint8_t RGB_G = 13;

/**
 * @brief RGB LED: Blau-Kanal.
 */
constexpr uint8_t RGB_B = 11;

// ==========================================================================
// 4. KOMMUNIKATION (BUS-SYSTEME)
// ==========================================================================

/**
 * @brief I2C Data Line (SDA).
 * @use   IMU (MPU6050), Port-Expander
 */
constexpr uint8_t I2C_SDA = A4;

/**
 * @brief I2C Clock Line (SCL).
 */
constexpr uint8_t I2C_SCL = A5;

} // namespace Pin
