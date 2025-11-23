/**
 * @file       Pins.h
 * @brief      Zentrale Definition aller GPIO Pin-Zuordnungen.
 * @details    Dieses Modul stellt die Schnittstelle zwischen logischen
 * Funktionen (z.B. MotorL_Forward) und den physikalischen Arduino Pin-Nummern
 * her. Dient der einfacheren Portabilität und Wartbarkeit.
 * @hardware   Arduino UNO / Microcontroller Board
 * @author     Jan Unger
 * @version    1.0.0
 * @date       2025-11-22
 */

/**
 * GalaxyRVR Shield – Pinbelegung (Arduino UNO)
 *
 *  Logischer Name       Arduino-Pin  Richtung      GalaxyRVR-Port / Hinweis
 *  -------------------------------------------------------------------------------
 *  MotorL_Forward       D2          Output/PWM    LEFT MOTOR, IN1 (H-Brücke)
 *  MotorL_Reverse       D3          Output/PWM    LEFT MOTOR, IN2 (H-Brücke)
 *  MotorR_Forward       D5          Output/PWM    RIGHT MOTOR, IN3 (H-Brücke)
 *  MotorR_Reverse       D4          Output/PWM    RIGHT MOTOR, IN4 (H-Brücke)
 *
 *  Servo                D6          Output/PWM    SERVO-Port (Kamera-Servo)
 *
 *  Ultrasonic_Trig      D10         Out/In        SONAR-Port, Trig/Echo
 * gemeinsam Ultrasonic_Echo      D10         Out/In        SONAR-Port,
 * Trig/Echo gemeinsam
 *
 *  RGB_R                D12         Output        RGB-Port, Rot-Kanal
 *  RGB_G                D13         Output        RGB-Port, Grün-Kanal
 * (Onboard-LED) RGB_B                D11         Output        RGB-Port,
 * Blau-Kanal
 *
 *  Hinweis:
 *  – Alle Motor-Pins werden aktuell per SoftPWM angesteuert.
 *  – D10 wird für das Ultraschallmodul im Code zwischen OUTPUT (Trig)
 *    und INPUT (Echo) umgeschaltet.
 */

#pragma once
#include <Arduino.h>

/**
 * @namespace Pin
 * @brief Enthält alle physischen GPIO-Pin-Nummern des Systems.
 * @details Die Verwendung eines dedizierten Namespaces verhindert
 * Konflikte mit anderen globalen Konstanten.
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

// TODO: Pin für den optionalen EN/Enable-Pin der H-Brücke hinzufügen, falls
// vorhanden.

// ----------------------------------------------------------------------
// SENSORIK & AKTORIK (ZUSÄTZLICHE KOMPONENTEN)
// ----------------------------------------------------------------------

/**
 * @brief  Pin für den Servo zur Kamerasteuerung.
 * @unit   GPIO Pin-Nummer
 */
constexpr uint8_t Servo = 6;

/**
 * @brief  Pin für den Ultraschall-Trigger (Sendet den Impuls).
 * @note   Im Originalskript identisch mit dem Echo-Pin (wird dynamisch
 * umgeschaltet).
 * @unit   GPIO Pin-Nummer
 */
constexpr uint8_t Ultrasonic_Trig = 10;

/**
 * @brief  Pin für den Ultraschall-Echo (Empfängt den Impuls).
 * @note   Im Originalskript identisch mit dem Trigger-Pin (wird dynamisch
 * umgeschaltet).
 * @unit   GPIO Pin-Nummer
 */
constexpr uint8_t Ultrasonic_Echo = 10;

/**
 * @brief  Pin für die R-Farbe der RGB-LED-Leiste.
 * @unit   GPIO Pin-Nummer
 */
constexpr uint8_t RGB_R = 12;

/**
 * @brief  Pin für die G-Farbe der RGB-LED-Leiste.
 * @note   Achtung: Dies ist Pin D13 (verbunden mit der Onboard-LED).
 * @unit   GPIO Pin-Nummer
 */
constexpr uint8_t RGB_G = 13;

/**
 * @brief  Pin für die B-Farbe der RGB-LED-Leiste.
 * @unit   GPIO Pin-Nummer
 */
constexpr uint8_t RGB_B = 11;

/**
 * @brief  Digitaler Ausgang des linken IR-Hindernisvermeidungsmoduls.
 * @note   LOW = Hindernis erkannt, HIGH = frei.
 */
constexpr uint8_t IR_Left = 8;

/**
 * @brief  Digitaler Ausgang des rechten IR-Hindernisvermeidungsmoduls.
 * @note   LOW = Hindernis erkannt, HIGH = frei.
 */
constexpr uint8_t IR_Right = 7;

constexpr uint8_t Battery = A3;

} // namespace Pin
