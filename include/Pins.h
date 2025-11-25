/**
 * @file       Pins.h
 * @brief      Zentrale Definition aller GPIO-Pin-Zuordnungen.
 *
 * @details
 * Stellt die Zuordnung zwischen logischen Funktionen (z. B.
 * @c MotorL_Forward ) und den physikalischen Arduino-Pin-Nummern her.
 * Dient der:
 *  - besseren Lesbarkeit,
 *  - einfacheren Portabilität,
 *  - konsistenten Anpassung bei Hardware-Änderungen.
 *
 * @hardware
 *  - Controller: Arduino Uno / SunFounder R3-kompatibel
 *  - Shield:    GalaxyRVR Shield
 *  - IMU:       Externer MPU6050 (GY-521) am I2C-Bus (A4/A5)
 *
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
 *  IR_Left               D8           Input         LEFT IR (Hindernis)
 *  IR_Right              D7           Input         RIGHT IR (Hindernis)
 *
 *  Battery               A3           Analog-In     Batteriespannung (2s-Pack)
 *
 *  I2C_SDA               A4           BiDir         I2C-Bus (Wire), z. B.
 * MPU6050 I2C_SCL               A5           BiDir         I2C-Bus (Wire), z.
 * B. MPU6050
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
 *
 * @details
 * Die Nutzung eines dedizierten Namespaces:
 *  - vermeidet Namenskonflikte mit anderen Konstanten,
 *  - macht die Herkunft der Pinzuordnung im Code sofort erkennbar,
 *  - erleichtert die Umstellung auf andere Boards/Pinbelegungen.
 */
namespace Pin {

// ----------------------------------------------------------------------
// MOTOR-TREIBER PIN-ZUORDNUNGEN (H-BRÜCKE)
// ----------------------------------------------------------------------

/**
 * @brief Steuerpin für die Vorwärtsbewegung des linken Motors.
 *
 * @details
 * Wird per (Soft-)PWM angesteuert und treibt IN1 der H-Brücke an.
 *
 * @unit Arduino-Digital-Pin (D2)
 */
constexpr uint8_t MotorL_Forward = 2;

/**
 * @brief Steuerpin für die Rückwärtsbewegung des linken Motors.
 *
 * @details
 * Wird per (Hard-)PWM angesteuert und treibt IN2 der H-Brücke an.
 *
 * @unit Arduino-Digital-Pin (D3)
 */
constexpr uint8_t MotorL_Reverse = 3;

/**
 * @brief Steuerpin für die Vorwärtsbewegung des rechten Motors.
 *
 * @details
 * Wird per (Hard-)PWM angesteuert und treibt IN3 der H-Brücke an.
 *
 * @unit Arduino-Digital-Pin (D5)
 */
constexpr uint8_t MotorR_Forward = 5;

/**
 * @brief Steuerpin für die Rückwärtsbewegung des rechten Motors.
 *
 * @details
 * Wird per (Soft-)PWM angesteuert und treibt IN4 der H-Brücke an.
 *
 * @unit Arduino-Digital-Pin (D4)
 */
constexpr uint8_t MotorR_Reverse = 4;

// TODO: Optionalen EN/Enable-Pin der H-Brücke hinzufügen, falls vorhanden.

// ----------------------------------------------------------------------
// SENSORIK & AKTORIK – SERVO, ULTRASCHALL, RGB, IR
// ----------------------------------------------------------------------

/**
 * @brief Pin für den Servo zur Kamerasteuerung.
 *
 * @details
 * PWM-fähiger Ausgang für den Kamera-Neigeservo am SERVO-Port.
 *
 * @unit Arduino-Digital-Pin (D6)
 */
constexpr uint8_t Servo = 6;

/**
 * @brief Pin für den Ultraschall-Trigger (Sendesignal).
 *
 * @details
 * - Sendet den 10-µs-Triggerpuls zum Auslösen der Messung.
 * - Im aktuellen Design identisch mit @c Ultrasonic_Echo und wird im Code
 *   dynamisch zwischen OUTPUT (Trig) und INPUT (Echo) umgeschaltet.
 *
 * @unit Arduino-Digital-Pin (D10)
 */
constexpr uint8_t Ultrasonic_Trig = 10;

/**
 * @brief Pin für den Ultraschall-Echo (Empfangssignal).
 *
 * @details
 * - Liest die Pulsdauer des zurückkehrenden Echos aus.
 * - Im aktuellen Design identisch mit @c Ultrasonic_Trig (Single-Pin-Betrieb).
 *
 * @unit Arduino-Digital-Pin (D10)
 */
constexpr uint8_t Ultrasonic_Echo = 10;

/**
 * @brief Pin für den Rot-Kanal der RGB-LED-Leiste.
 *
 * @unit Arduino-Digital-Pin (D12)
 */
constexpr uint8_t RGB_R = 12;

/**
 * @brief Pin für den Grün-Kanal der RGB-LED-Leiste.
 *
 * @details
 * Achtung: Dieser Pin ist identisch mit der Onboard-LED des Arduino Uno
 * (L-LED). Ein Blinken der Onboard-LED beeinflusst somit den Grün-Kanal.
 *
 * @unit Arduino-Digital-Pin (D13)
 */
constexpr uint8_t RGB_G = 13;

/**
 * @brief Pin für den Blau-Kanal der RGB-LED-Leiste.
 *
 * @unit Arduino-Digital-Pin (D11)
 */
constexpr uint8_t RGB_B = 11;

/**
 * @brief Digitaler Ausgang des linken IR-Hindernissensors.
 *
 * @details
 * Logikpegel:
 *  - LOW  = Hindernis erkannt,
 *  - HIGH = Bahn frei.
 *
 * @unit Arduino-Digital-Pin (D8)
 */
constexpr uint8_t IR_Left = 8;

/**
 * @brief Digitaler Ausgang des rechten IR-Hindernissensors.
 *
 * @details
 * Logikpegel:
 *  - LOW  = Hindernis erkannt,
 *  - HIGH = Bahn frei.
 *
 * @unit Arduino-Digital-Pin (D7)
 */
constexpr uint8_t IR_Right = 7;

// ----------------------------------------------------------------------
// ENERGIESYSTEM – BATTERIEMESSUNG
// ----------------------------------------------------------------------

/**
 * @brief Analoger Eingang für die Batteriespannungsmessung.
 *
 * @details
 * Misst die Spannung eines 2s-Li-Ion-Packs über einen Spannungsteiler.
 * Die Umrechnung von ADC-Rohwert in Volt erfolgt in
 * @c HAL::Sensor::getBatteryVoltage() .
 *
 * @unit Arduino-Analog-Pin (A3)
 */
constexpr uint8_t Battery = A3;

// ----------------------------------------------------------------------
// BUS-SIGNALE – I2C (IMU, zusätzliche Sensoren)
// ----------------------------------------------------------------------

/**
 * @brief I2C-Datenleitung (SDA).
 *
 * @details
 * Entspricht A4 des Arduino Uno. Wird vom Wire-Treiber genutzt, z. B. für:
 *  - MPU6050 (GY-521),
 *  - weitere I2C-Sensoren (Magnetometer, Kamera-Adapter, etc.).
 *
 * @unit Arduino-Analog-Pin (A4)
 */
constexpr uint8_t I2C_SDA = A4;

/**
 * @brief I2C-Taktsignal (SCL).
 *
 * @details
 * Entspricht A5 des Arduino Uno. Wird vom Wire-Treiber genutzt, z. B. für:
 *  - MPU6050 (GY-521),
 *  - weitere I2C-Slaves am I2C-Bus.
 *
 * @unit Arduino-Analog-Pin (A5)
 */
constexpr uint8_t I2C_SCL = A5;

} // namespace Pin
