/**
 * @file    main.cpp
 * @brief   Phase 1 Abschluss: Die "Quadrat-Mission".
 * @details Testet Odometrie (50cm) und Gyro (90°) im Zusammenspiel.
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

// Seitenlänge des Quadrats
constexpr float SIDE_LENGTH_CM = 50.0f;
// Nachlauf-Korrektur für Drehungen
constexpr float TURN_INERTIA_DEG = 2.5f;

// ==========================================================================
// GLOBALE INSTANZEN
// ==========================================================================

Logic::Odometry odometry;
float globalYawAngle = 0.0f;

// Zustände für unsere Mission
enum class MissionState {
    LEG_1,
    TURN_1,
    LEG_2,
    TURN_2,
    LEG_3,
    TURN_3,
    LEG_4,
    DONE
};

MissionState currentState = MissionState::LEG_1;

// Start-Koordinaten für das aktuelle Segment
float startX = 0.0f;
float startY = 0.0f;

// ==========================================================================
// HELPER
// ==========================================================================

void performTurn(float targetAngleDeg) {
    Serial.print(F(">>> DREHUNG: "));
    Serial.println(targetAngleDeg);
    HAL::Motor::stop();
    delay(500);

    float currentAngle = 0.0f;
    unsigned long lastTime = millis();

    HAL::Motor::setSpeed(-TURN_SPEED, TURN_SPEED); // Links drehen

    while (abs(currentAngle) < (targetAngleDeg - TURN_INERTIA_DEG)) {
        HAL::Sensor::imuUpdate();
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0f;
        lastTime = now;

        float rate = HAL::Sensor::getYawRate();
        currentAngle += rate * dt;

        if (abs(rate) > 0.3f)
            globalYawAngle += rate * dt;

        odometry.update(globalYawAngle, dt, HAL::Sensor::getBatteryVoltage(),
                        false);
    }
    HAL::Motor::stop();
    delay(1000);
    Logic::DriveAssistant::init();

    // Startpunkt für das nächste Segment neu setzen
    startX = odometry.getX();
    startY = odometry.getY();
}

// Berechnet Distanz seit Start des aktuellen Segments (Pythagoras)
float getDistanceDriven() {
    float dx = odometry.getX() - startX;
    float dy = odometry.getY() - startY;
    return sqrt(dx * dx + dy * dy);
}

// ==========================================================================
// MAIN
// ==========================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("=== MISSION: QUADRAT FAHREN ==="));

    HAL::Sensor::init();
    HAL::Motor::init();
    HAL::Actuator::init();
    Logic::DriveAssistant::init();
    odometry.reset();

    Serial.println(F("IMU: Kalibrierung..."));
    HAL::Actuator::setCameraAngle(90);
    HAL::Sensor::imuCalibrate(100);

    Serial.println(F("--> START in 3 Sekunden..."));
    delay(3000);

    startX = odometry.getX();
    startY = odometry.getY();
}

void loop() {
    // 1. Zeit & Physik Update
    static unsigned long lastLoopTime = 0;
    unsigned long now = millis();
    float dt = (now - lastLoopTime) / 1000.0f;
    lastLoopTime = now;
    if (dt > 0.1f)
        dt = 0.0f;

    HAL::Sensor::imuUpdate();
    float yawRate = HAL::Sensor::getYawRate();
    if (abs(yawRate) > 0.3f)
        globalYawAngle += yawRate * dt;

    // Odometrie läuft immer mit, wenn wir nicht im Turn sind (dort manuell)
    // Wir übergeben 'true' (moving), es sei denn wir sind fertig.
    bool isActive = (currentState != MissionState::DONE);
    odometry.update(globalYawAngle, dt, HAL::Sensor::getBatteryVoltage(),
                    isActive);

    // 2. State Machine
    switch (currentState) {

    // --- GERADEAUS-PHASEN ---
    case MissionState::LEG_1:
    case MissionState::LEG_2:
    case MissionState::LEG_3:
    case MissionState::LEG_4:
        Logic::DriveAssistant::update(CRUISE_SPEED);

        if (getDistanceDriven() >= SIDE_LENGTH_CM) {
            HAL::Motor::stop();
            Serial.println(F("Schenkel fertig."));

            // Zustands-Wechsel
            if (currentState == MissionState::LEG_1)
                currentState = MissionState::TURN_1;
            else if (currentState == MissionState::LEG_2)
                currentState = MissionState::TURN_2;
            else if (currentState == MissionState::LEG_3)
                currentState = MissionState::TURN_3;
            else if (currentState == MissionState::LEG_4)
                currentState = MissionState::DONE;
        }
        break;

    // --- DREH-PHASEN ---
    case MissionState::TURN_1:
        performTurn(90.0f);
        currentState = MissionState::LEG_2;
        break;
    case MissionState::TURN_2:
        performTurn(90.0f);
        currentState = MissionState::LEG_3;
        break;
    case MissionState::TURN_3:
        performTurn(90.0f);
        currentState = MissionState::LEG_4;
        break;

    // --- ENDE ---
    case MissionState::DONE:
        HAL::Motor::stop();
        // Optional: LED blinken oder Kamera wackeln als Jubel
        break;
    }

    // Debug Ausgabe
    static unsigned long lastPrint = 0;
    if (now - lastPrint > 500) {
        lastPrint = now;
        Serial.print(F("State: "));
        Serial.print((int)currentState);
        Serial.print(F(" | SegDist: "));
        Serial.print(getDistanceDriven());
        Serial.print(F(" | X:"));
        Serial.print(odometry.getX());
        Serial.print(F(" Y:"));
        Serial.println(odometry.getY());
    }

    delay(5);
}
