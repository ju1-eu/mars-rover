/**
 * @file    main.cpp
 * @brief   Finale Firmware Phase 1: PID-Cruise + Odometrie + Obstacle
 * Avoidance.
 * @version 1.4.0 (Phase 1 Gold)
 * @date    2025-11-25
 *
 * @details
 * Kombiniert alle Features aus Phase 1:
 * - PID-Regelung (mit Bias-Trimm) für geraden Lauf.
 * - Odometrie (Koppelnavigation) zur Positionsbestimmung.
 * - Inertia-Kompensierte 180° Wenden bei Hindernissen.
 */

#include "Config.h"
#include "hal/Actuator.h"
#include "hal/Motor.h"
#include "hal/Sensor.h"
#include "logic/DriveAssistant.h"
#include "logic/Odometry.h"
#include <Arduino.h>

// ==========================================================================
// KONFIGURATION
// ==========================================================================

constexpr uint8_t CRUISE_SPEED = 100;
constexpr uint8_t TURN_SPEED = 80;
constexpr float STOP_DISTANCE_CM = 15.0f;

// ==========================================================================
// GLOBALE INSTANZEN
// ==========================================================================

Logic::Odometry odometry;
float globalYawAngle = 0.0f;

// ==========================================================================
// HILFSFUNKTIONEN
// ==========================================================================

/**
 * @brief Führt eine 180° Wende aus und aktualisiert dabei die Odometrie.
 */
void performTurn180() {
    Serial.println(F(">>> MANÖVER: Wende (180°) <<<"));
    HAL::Motor::stop();
    delay(500);

    // Wir wollen 180 Grad drehen.
    // Stoppen bei 177.5 Grad (2.5 Grad Trägheit / Inertia)
    const float targetDelta = 180.0f;
    const float stopThreshold = targetDelta - 2.5f;

    float turnedAmount = 0.0f;
    unsigned long lastTime = millis();

    HAL::Motor::setSpeed(-TURN_SPEED, TURN_SPEED);

    while (abs(turnedAmount) < stopThreshold) {
        HAL::Sensor::imuUpdate();
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;

        float rate = HAL::Sensor::getYawRate();

        // 1. Fortschritt der Drehung messen
        turnedAmount += rate * dt;

        // 2. Globalen Winkel mitführen (wichtig für Rückweg!)
        if (abs(rate) > 0.3f)
            globalYawAngle += rate * dt;

        // 3. Odometrie updaten (im Stand, nur Winkel ändert sich)
        odometry.update(globalYawAngle, dt, HAL::Sensor::getBatteryVoltage(),
                        false);

        // Safety check
        if (abs(HAL::Sensor::getPitch()) > 60.0f) {
            HAL::Motor::stop();
            return;
        }
    }

    HAL::Motor::stop();
    delay(1000);

    // Regler resetten, da wir jetzt in die Gegenrichtung schauen
    Logic::DriveAssistant::init();
}

// ==========================================================================
// SETUP & LOOP
// ==========================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("=== GALAXY RVR: PHASE 1 FINAL ==="));

    HAL::Sensor::init();
    HAL::Motor::init();
    HAL::Actuator::init();
    Logic::DriveAssistant::init();
    odometry.reset();

    Serial.println(F("IMU: Kalibrierung..."));
    HAL::Actuator::setCameraAngle(90);
    HAL::Sensor::imuCalibrate(100);

    Serial.println(F("--> System bereit. Start in 2s..."));
    delay(2000);
}

void loop() {
    // 1. Zeit & Physik
    static unsigned long lastLoopTime = 0;
    unsigned long now = millis();
    float dt = (now - lastLoopTime) / 1000.0f;
    lastLoopTime = now;
    if (dt > 0.1f)
        dt = 0.0f;

    HAL::Sensor::imuUpdate();
    HAL::Sensor::ultrasonicUpdate();

    // Globalen Winkel tracken
    float yawRate = HAL::Sensor::getYawRate();
    if (abs(yawRate) > 0.3f)
        globalYawAngle += yawRate * dt;

    // 2. Logik
    float dist = HAL::Sensor::getUltrasonicDistance();

    if (dist > 0.1f && dist < STOP_DISTANCE_CM) {
        // Hindernis -> Wenden
        performTurn180();
        lastLoopTime = millis(); // Timer reset nach blockierendem Manöver
    } else {
        // Freie Fahrt -> PID Cruise
        bool safe = Logic::DriveAssistant::update(CRUISE_SPEED);
        if (!safe) {
            HAL::Motor::stop();
            while (true)
                delay(100); // Not-Halt
        }

        // Odometrie: Wir fahren (isMoving = true)
        odometry.update(globalYawAngle, dt, HAL::Sensor::getBatteryVoltage(),
                        true);
    }

    // 3. Telemetrie (alle 1s reicht hier)
    static unsigned long lastPrint = 0;
    if (now - lastPrint > 1000) {
        lastPrint = now;
        Serial.print(F("[POS] X:"));
        Serial.print(odometry.getX(), 0);
        Serial.print(F(" Y:"));
        Serial.print(odometry.getY(), 0);
        Serial.print(F(" | Bat:"));
        Serial.print(HAL::Sensor::getBatteryVoltage(), 1);
        Serial.println(F("V"));
    }

    delay(5);
}
