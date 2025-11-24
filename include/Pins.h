/**
 * @file       Pins.h
 * @brief      Zentrale Definition aller GPIO-Pin-Zuordnungen.
 * @details    Stellt die Zuordnung zwischen logischen Funktionen
 *             (z. B. MotorL_Forward) und den physikalischen Arduino-Pin-Nummern
 *             her. Dient der einfacheren Portabilität und Wartbarkeit.
 * @hardware   Arduino Uno + GalaxyRVR Shield (SunFounder R3 kompatibel)
 *             + externer IMU-Sensor (MPU6050, GY-521) am I2C-Bus (A4/A5)
 * @author     Jan Unger
 * @version    1.0.2
 * @date       2025-11-24
 */

/**
 * GalaxyRVR Shield – Pinbelegung (Arduino UNO)
 *
 *  Logischer Name        Arduino-Pin  Richtung      GalaxyRVR-Port / Hinweis
 *  ---------------------------------------------------------------------------
 *  MotorL_Forward        D2           Output/PWM    LEFT MOTOR, IN1 (H-Brücke)
 *  MotorL_Reverse        D3           Output/PWM    LEFT MOTOR, IN2 (H-Brücke)
 *  MotorR_Forward        D5           Output/PWM    RIGHT MOTOR, IN3 (H-Brücke)
 *  MotorR_Reverse        D4           Output/PWM    RIGHT MOTOR, IN4 (H-Brücke)
 *
 *  Servo                 D6           Output/PWM    SERVO-Port (Kamera-Servo)
 *
 *  Ultrasonic_Trig       D10          Out/In        SONAR-Port, Trig/Echo
 *  Ultrasonic_Echo       D10          Out/In        SONAR-Port, Trig/Echo
 *
 *  RGB_R                 D12          Output        RGB-Port, Rot-Kanal
 *  RGB_G                 D13          Output        RGB-Port, Grün-Kanal
 *  RGB_B                 D11          Output        RGB-Port, Blau-Kanal
 *
 *  IR_Left               D8           Input         LEFT IR
 * (Hindernisvermeidung) IR_Right              D7           Input         RIGHT
 * IR (Hindernisvermeidung)
 *
 *  Battery               A3           Analog-In     Batteriespannung (2s-Pack)
 *
 *  I2C_SDA               A4           BiDir         I2C-Bus (Wire), z. B.
 * MPU6050 GY-521 I2C_SCL               A5           BiDir         I2C-Bus
 * (Wire), z. B. MPU6050 GY-521
 *
 *  Hinweise:
 *  – Alle Motor-Pins werden aktuell per SoftPWM angesteuert.
 *  – D10 wird für das Ultraschallmodul im Code zwischen OUTPUT (Trig)
 *    und INPUT (Echo) umgeschaltet.
 *  – D13 ist zusätzlich mit der Onboard-LED des Uno verbunden.
 *  – Der I2C-Bus (A4/A5) wird vom Wire-Treiber verwendet; zusätzliche
 *    Sensoren (IMU, Magnetometer, Kamera-Adapter) können hier angebunden
 * werden.
 */

#pragma once

#include <Arduino.h> // definiert uint8_t, Pin-Konstanten (A0, A1, ...)

/**
 * @namespace Pin
 * @brief Enthält alle physischen GPIO-Pin-Nummern des Systems.
 * @details Die Verwendung eines dedizierten Namespaces verhindert
 *          Konflikte mit anderen globalen Konstanten.
 */
namespace Pin {

// ----------------------------------------------------------------------
// MOTOR-TREIBER PIN-ZUORDNUNGEN (H-BRÜCKE)
// ----------------------------------------------------------------------

/** @brief Steuerpin für die Vorwärtsbewegung des linken Motors (SoftPWM). */
constexpr uint8_t MotorL_Forward = 2;

/** @brief Steuerpin für die Rückwärtsbewegung des linken Motors (HardPWM). */
constexpr uint8_t MotorL_Reverse = 3;

/** @brief Steuerpin für die Vorwärtsbewegung des rechten Motors (HardPWM). */
constexpr uint8_t MotorR_Forward = 5;

/** @brief Steuerpin für die Rückwärtsbewegung des rechten Motors (SoftPWM). */
constexpr uint8_t MotorR_Reverse = 4;

// TODO: Optionalen EN/Enable-Pin der H-Brücke hinzufügen, falls vorhanden.

// ----------------------------------------------------------------------
// SENSORIK & AKTORIK – SERVO, ULTRASCHALL, RGB, IR
// ----------------------------------------------------------------------

/**
 * @brief Pin für den Servo zur Kamerasteuerung.
 * @unit  GPIO-Pin-Nummer (Digital-Pin)
 */
constexpr uint8_t Servo = 6;

/**
 * @brief Pin für den Ultraschall-Trigger (sendet den Impuls).
 * @note  Im aktuellen Design identisch mit dem Echo-Pin (wird dynamisch
 *        zwischen OUTPUT und INPUT umgeschaltet).
 */
constexpr uint8_t Ultrasonic_Trig = 10;

/**
 * @brief Pin für den Ultraschall-Echo (empfängt den Impuls).
 * @note  Im aktuellen Design identisch mit dem Trigger-Pin (wird dynamisch
 *        zwischen OUTPUT und INPUT umgeschaltet).
 */
constexpr uint8_t Ultrasonic_Echo = 10;

/**
 * @brief Pin für die R-Komponente der RGB-LED-Leiste.
 */
constexpr uint8_t RGB_R = 12;

/**
 * @brief Pin für die G-Komponente der RGB-LED-Leiste.
 * @note  Achtung: Dies ist Pin D13 (verbunden mit der Onboard-LED).
 */
constexpr uint8_t RGB_G = 13;

/**
 * @brief Pin für die B-Komponente der RGB-LED-Leiste.
 */
constexpr uint8_t RGB_B = 11;

/**
 * @brief Digitaler Ausgang des linken IR-Hindernisvermeidungsmoduls.
 * @note  LOW = Hindernis erkannt, HIGH = frei.
 */
constexpr uint8_t IR_Left = 8;

/**
 * @brief Digitaler Ausgang des rechten IR-Hindernisvermeidungsmoduls.
 * @note  LOW = Hindernis erkannt, HIGH = frei.
 */
constexpr uint8_t IR_Right = 7;

// ----------------------------------------------------------------------
// ENERGIESYSTEM – BATTERIEMESSUNG
// ----------------------------------------------------------------------

/**
 * @brief Analoger Eingang für die Batteriespannungsmessung.
 * @details Liest die Spannung eines 2s-Li-Ion-Packs über einen Spannungsteiler.
 *          Die Umrechnung auf Volt erfolgt in HAL::Sensor::getBatteryVoltage().
 */
constexpr uint8_t Battery = A3;

// ----------------------------------------------------------------------
// BUS-SIGNALE – I2C (IMU, zusätzliche Sensoren)
// ----------------------------------------------------------------------

/**
 * @brief I2C-Datenleitung (SDA).
 * @details Entspricht A4 des Arduino Uno. Wird vom Wire-Treiber genutzt,
 *          z. B. für MPU6050 (GY-521) und weitere I2C-Sensoren.
 */
constexpr uint8_t I2C_SDA = A4;

/**
 * @brief I2C-Taktsignal (SCL).
 * @details Entspricht A5 des Arduino Uno. Wird vom Wire-Treiber genutzt,
 *          z. B. für MPU6050 (GY-521) und weitere I2C-Sensoren.
 */
constexpr uint8_t I2C_SCL = A5;

} // namespace Pin
