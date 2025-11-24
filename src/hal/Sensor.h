/**
 * @file    Sensor.h
 * @brief   Schnittstelle für Sensor-HAL (Ultraschall, IR, Batterie, IMU).
 * @author  Jan Unger
 */

#pragma once

#include <Arduino.h>

namespace HAL::Sensor {

// Basis-Initialisierung
void init();

// Ultraschall
void ultrasonicTest();

// IR-Sensoren
bool irLeftBlocked();
bool irRightBlocked();
void irTest();

// Batterie
float getBatteryVoltage();
uint8_t getBatteryPercentage();
bool isBatteryCharging();

// ===== IMU / MPU6050 (GY-521) =====

// Initialisierung & Kalibrierung (im Stillstand aufrufen!)
void imuCalibrate(uint16_t samples = 500);

// Haupt-Funktion: Muss im loop() ständig gerufen werden!
// Berechnet Pitch/Roll/YawRate neu (Sensor-Fusion).
void imuUpdate();

// Getter: Damit die Motorsteuerung die Werte lesen kann
float getPitch();   // Nase hoch/runter [Grad]
float getRoll();    // Seitlich kippen [Grad]
float getYawRate(); // Drehgeschwindigkeit [Grad/s]

// Diagnose: Gibt die aktuellen Werte lesbar aus (langsam)
void imuTest();

} // namespace HAL::Sensor
