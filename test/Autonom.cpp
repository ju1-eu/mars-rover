/**
 * @file    main.cpp
 * @brief   Autonomer Modus: Geradeausfahrt + 180°-Wende bei Hindernis.
 */

#include "Config.h"
#include "hal/Actuator.h"
#include "hal/Motor.h"
#include "hal/Sensor.h"
#include "logic/DriveAssistant.h"
#include <Arduino.h>

// --- EINSTELLUNGEN ---
constexpr uint8_t CRUISE_SPEED = 100; // 38 für Test/Tisch, 80-100 für Boden
constexpr uint8_t TURN_SPEED = 80;    // Kraftvolles Drehen für die Wende
constexpr float STOP_DISTANCE_CM = 15.0f;

// Hilfsfunktion: Führt eine überwachte 180-Grad-Wende durch
void performTurn180() {
    Serial.println(F("--- WENDE MANOEVER START (180 Grad) ---"));
    HAL::Motor::stop();
    delay(500);

    float currentAngle = 0.0f;
    unsigned long lastTime = millis();

    // Drehung einleiten (Linksherum: Links Rückwärts, Rechts Vorwärts)
    HAL::Motor::setSpeed(-TURN_SPEED, TURN_SPEED);

    // Schleife: Solange drehen, bis 180 Grad erreicht sind
    while (abs(currentAngle) < 180.0f) {
        // 1. Sensoren wach halten
        HAL::Sensor::imuUpdate();

        // 2. Zeit-Delta berechnen
        unsigned long now = millis();
        float dt = (now - lastTime) / 1000.0f; // in Sekunden
        lastTime = now;

        // 3. Winkel integrieren (Rate * Zeit = Winkeländerung)
        // getYawRate liefert °/s.
        float rate = HAL::Sensor::getYawRate();
        currentAngle += rate * dt;

        // Debug-Ausgabe alle 100ms
        static unsigned long lastPrint = 0;
        if (now - lastPrint > 100) {
            lastPrint = now;
            Serial.print(F("Wende: "));
            Serial.print(abs(currentAngle), 0);
            Serial.println(F(" / 180 Grad"));
        }

        // Not-Aus bei Kippen (Sicherheit)
        if (abs(HAL::Sensor::getPitch()) > 60.0f) {
            HAL::Motor::stop();
            return;
        }
    }

    HAL::Motor::stop();
    Serial.println(F("--- WENDE ABGESCHLOSSEN ---"));
    delay(1000); // Kurz orientieren vor Weiterfahrt

    // Assistenten resetten, damit er nicht wegen des Drehens gegenlenkt
    Logic::DriveAssistant::init();
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("--- GALAXY RVR: AUTONOMER MODUS + WENDE ---"));

    HAL::Sensor::init();
    HAL::Motor::init();
    HAL::Actuator::init();
    Logic::DriveAssistant::init();

    Serial.println(F("IMU: Kalibrierung... (Stillhalten!)"));
    HAL::Actuator::setCameraAngle(90);
    HAL::Sensor::imuCalibrate(50);
    Serial.println(F("IMU: Bereit."));

    Serial.println(F("START IN 3 SEKUNDEN..."));
    delay(3000);
}

void loop() {
    // Sensoren immer aktualisieren
    HAL::Sensor::imuUpdate();
    HAL::Sensor::ultrasonicUpdate();

    float distance = HAL::Sensor::getUltrasonicDistance();

    // --- HINDERNIS-LOGIK ---
    if (distance > 0.1f && distance < STOP_DISTANCE_CM) {
        // Hindernis erkannt!
        Serial.print(F("HINDERNIS BEI "));
        Serial.print(distance, 0);
        Serial.println(F(" cm -> START WENDE!"));

        performTurn180(); // Blockierende Funktion, kehrt erst nach Wende zurück
    } else {
        // --- FREIE FAHRT ---
        bool safe = Logic::DriveAssistant::update(CRUISE_SPEED);

        if (!safe) {
            HAL::Motor::stop();
            Serial.println(F("NOT-AUS: Rover gekippt!"));
            while (true)
                delay(100);
        }
    }

    // Batterie-Check alle 5s
    static unsigned long lastBat = 0;
    if (millis() - lastBat > 5000) {
        lastBat = millis();
        Serial.print(F("[Bat] "));
        Serial.print(HAL::Sensor::getBatteryVoltage());
        Serial.println(F(" V"));
    }
}
